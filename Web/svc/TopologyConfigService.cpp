#include "Web/svc/TopologyConfigService.hpp"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

namespace {
  const char* const TOPOLOGY_CONFIG_PATH = "../data/config/topology_config.json";
}

TopologyConfigService& TopologyConfigService::instance() {
  static TopologyConfigService service;
  return service;
}

TopologyConfigService::TopologyConfigService()
  : loaded_(false)
  , objectMapper_(oatpp::parser::json::mapping::ObjectMapper::createShared()) {
}

oatpp::Object<TopologyConfigDto> TopologyConfigService::getConfig() {
  std::lock_guard<std::mutex> lock(mutex_);
  ensureLoadedLocked();
  return cloneConfig(config_);
}

oatpp::Object<TopologyConfigDto> TopologyConfigService::updateConfig(const oatpp::Object<TopologyConfigDto>& dto) {
  std::lock_guard<std::mutex> lock(mutex_);
  ensureLoadedLocked();
  if (!dto) {
    return cloneConfig(config_);
  }

  auto copy = cloneConfig(dto);
  normalizeConfigLocked(copy);
  config_ = copy;
  persistLocked();
  return cloneConfig(config_);
}

void TopologyConfigService::ensureLoadedLocked() {
  if (!loaded_) {
    loadLocked();
  }
}

void TopologyConfigService::loadLocked() {
  std::ifstream file(TOPOLOGY_CONFIG_PATH);
  if (file.good()) {
    std::stringstream buffer;
    buffer << file.rdbuf();
    auto content = buffer.str();
    if (!content.empty()) {
      try {
        auto dto = objectMapper_->readFromString<oatpp::Object<TopologyConfigDto>>(content.c_str());
        normalizeConfigLocked(dto);
        config_ = dto;
        loaded_ = true;
        return;
      } catch (const std::exception& ex) {
        OATPP_LOGE("TopologyConfigService", "Failed to parse topology config file: %s", ex.what());
      } catch (...) {
        OATPP_LOGE("TopologyConfigService", "Failed to parse topology config file: unknown error");
      }
    }
  } else {
    OATPP_LOGD("TopologyConfigService", "Topology config file %s not found. Using defaults.", TOPOLOGY_CONFIG_PATH);
  }

  config_ = makeDefaultConfigLocked();
  persistLocked();
  loaded_ = true;
}

void TopologyConfigService::persistLocked() {
  if (!config_) {
    return;
  }
  namespace fs = std::filesystem;
  fs::path path(TOPOLOGY_CONFIG_PATH);
  std::error_code ec;
  fs::create_directories(path.parent_path(), ec);
  if (ec) {
    OATPP_LOGE("TopologyConfigService", "Failed to create directory %s: %s", path.parent_path().c_str(), ec.message().c_str());
  }
  auto json = objectMapper_->writeToString(config_);
  std::ofstream out(path);
  if (!out.is_open()) {
    OATPP_LOGE("TopologyConfigService", "Failed to open topology config file for writing: %s", path.c_str());
    return;
  }
  if (json) {
    out << json->c_str();
  } else {
    out << "{}";
  }
}

oatpp::Object<TopologyConfigDto> TopologyConfigService::makeDefaultConfigLocked() {
  auto config = TopologyConfigDto::createShared();
  config->subdomainCount = 1;
  auto list = oatpp::List<oatpp::Object<SubdomainConfigDto>>::createShared();
  auto subdomain = SubdomainConfigDto::createShared();
  subdomain->subdomainId = uint32_t(0);
  subdomain->nodeCount = uint32_t(0);
  subdomain->linkCount = uint32_t(0);
  list->push_back(subdomain);
  config->subdomains = list;
  // TODO: attach controllers per subdomain alongside topology counts.
  return config;
}

oatpp::Object<TopologyConfigDto> TopologyConfigService::cloneConfig(const oatpp::Object<TopologyConfigDto>& dto) const {
  if (!dto) {
    return nullptr;
  }
  auto payload = objectMapper_->writeToString(dto);
  if (!payload) {
    return nullptr;
  }
  return objectMapper_->readFromString<oatpp::Object<TopologyConfigDto>>(payload);
}

void TopologyConfigService::normalizeConfigLocked(const oatpp::Object<TopologyConfigDto>& dto) {
  if (!dto) {
    return;
  }

  auto list = dto->subdomains;
  if (!list) {
    list = oatpp::List<oatpp::Object<SubdomainConfigDto>>::createShared();
  }

  v_uint32 desired = dto->subdomainCount.getValue(list->size());

  if (list->size() > desired) {
    auto trimmed = oatpp::List<oatpp::Object<SubdomainConfigDto>>::createShared();
    v_uint32 idx = 0;
    for (auto& item : *list) {
      if (idx++ >= desired) {
        break;
      }
      trimmed->push_back(item ? item : SubdomainConfigDto::createShared());
    }
    list = trimmed;
  }

  while (list->size() < desired) {
    auto subdomain = SubdomainConfigDto::createShared();
    subdomain->subdomainId = list->size();
    subdomain->nodeCount = uint32_t(0);
    subdomain->linkCount = uint32_t(0);
    list->push_back(subdomain);
  }

  v_uint32 idx = 0;
  for (auto& item : *list) {
    if (!item) {
      item = SubdomainConfigDto::createShared();
    }
    item->subdomainId = idx;
    item->nodeCount = item->nodeCount.getValue(0);
    item->linkCount = item->linkCount.getValue(0);
    ++idx;
  }

  dto->subdomains = list;
  dto->subdomainCount = list->size();
}

static std::string toStdString(const oatpp::String& s) {
  return s ? std::string((const char*)s->data(), s->size()) : std::string();
}

std::string TopologyConfigService::getSavedPath(
  const oatpp::Object<TopologyGenerateRequestDto>& req) const {
  // Python 脚本会把 network.csv/demand.csv 写到 out_dir
  // 这里给前端一个直观路径
  std::string outDir = toStdString(req->out_dir);
  if(outDir.empty()) outDir = "data";
  // 仅作返回提示，不强制与 filename 绑定（python 实际固定写 network.csv/demand.csv）
  return outDir + "/networks/"+req->filename.getValue("");
}

// 读取管道到字符串
static std::string readAllFromFd(int fd) {
  std::string buf;
  char tmp[4096];
  ssize_t n;
  while ((n = ::read(fd, tmp, sizeof(tmp))) > 0) {
    buf.append(tmp, tmp + n);
  }
  return buf;
}

// 构建 argv 向量（不通过 shell，避免转义问题）
int TopologyConfigService::runPythonDomainGen(
      const std::string& pythonBin,
      const std::string& scriptPath,
      const oatpp::Object<TopologyGenerateRequestDto>& req,
      std::string& out,
      std::string& err) {

  // 1) 用 Oat++ JSON 映射器把 domains / inter_edges 序列化为 JSON 字符串
  auto mapper = oatpp::parser::json::mapping::ObjectMapper::createShared();

  // 序列化 domains（List<Object<DomainSpecDto>>）
  oatpp::String domainsJson = mapper->writeToString(req->domains);
  // 序列化 inter_edges（List<List<Int32>>）
  oatpp::String interJson   = mapper->writeToString(req->inter_edges);
  auto filename=req->filename.getValue("");

  // 2) 组装 argv
  std::vector<std::string> argsStr = {
    pythonBin,
    scriptPath,
    "--filename",filename,
    "--domains",       toStdString(domainsJson),
    "--inter-edges",   toStdString(interJson),
    "--seed",          std::to_string(req->seed ? *req->seed : 42),
    "--num-pairs",     std::to_string(req->num_pairs ? *req->num_pairs : 1),
    "--intra-pair-ratio", std::to_string(req->intra_pair_ratio ? *req->intra_pair_ratio : 0.8f),
    "--max-pair-hops", std::to_string(req->max_pair_hops ? *req->max_pair_hops : 8),
    "--plot-limit",    "1",
    "--out-dir",       "../data"
    // "--plot-limit",    std::to_string(req->plot_limit ? *req->plot_limit : 1000),
    // "--out-dir",       toStdString(req->out_dir)
  };

  // 注意：python 脚本里并没有 --filename 这个参数；如果你想让脚本用 filename 决定输出文件名，
  // 可以自行扩展脚本的 argparse。这里我们只把 filename 交给 C++ 层做展示，不传给 python。
  // 如果你已经扩展了脚本参数，可以在此追加：
  // argsStr.push_back("--filename");
  // argsStr.push_back(toStdString(req->filename));

  // 转换为 execvp 需要的 argv**
  std::vector<char*> argv;
  argv.reserve(argsStr.size() + 1);
  for (auto& s : argsStr) argv.push_back(const_cast<char*>(s.c_str()));
  argv.push_back(nullptr);

  // 3) 建立 stdout/stderr 管道
  int outPipe[2], errPipe[2];
  if (pipe(outPipe) == -1) return -1;
  if (pipe(errPipe) == -1) { close(outPipe[0]); close(outPipe[1]); return -1; }

  pid_t pid = fork();
  if (pid == -1) {
    close(outPipe[0]); close(outPipe[1]);
    close(errPipe[0]); close(errPipe[1]);
    return -1;
  }

  if (pid == 0) {
    // 子进程：重定向 stdout/stderr 到管道，关闭读端
    dup2(outPipe[1], STDOUT_FILENO);
    dup2(errPipe[1], STDERR_FILENO);
    close(outPipe[0]); close(outPipe[1]);
    close(errPipe[0]); close(errPipe[1]);

    // 执行 python
    execvp(argv[0], argv.data());
    // 如果 execvp 失败
    _exit(127);
  }

  // 父进程：关闭写端，读取输出
  close(outPipe[1]);
  close(errPipe[1]);

  // 非阻塞可选；这里简单阻塞读完
  out = readAllFromFd(outPipe[0]);
  err = readAllFromFd(errPipe[0]);

  close(outPipe[0]);
  close(errPipe[0]);

  int status = 0;
  waitpid(pid, &status, 0);

  int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
  return exitCode;
}