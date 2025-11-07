#ifndef TOPOLOGYCONFIGSERVICE_HPP
#define TOPOLOGYCONFIGSERVICE_HPP

#include <mutex>
#include <memory>

#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "Web/dto/DTOs.hpp"

class TopologyConfigService {
public:
    static TopologyConfigService& instance();

    oatpp::Object<TopologyConfigDto> getConfig();
    oatpp::Object<TopologyConfigDto> updateConfig(const oatpp::Object<TopologyConfigDto>& dto);
     int runPythonDomainGen(
      const std::string& pythonBin,
      const std::string& scriptPath,
      const oatpp::Object<TopologyGenerateRequestDto>& req,
      std::string& out,
      std::string& err);

    // 你也可以在这里做落地文件名的决定
    std::string getSavedPath(const oatpp::Object<TopologyGenerateRequestDto>& req) const;

private:
    TopologyConfigService();

    void ensureLoadedLocked();
    void loadLocked();
    void persistLocked();
    oatpp::Object<TopologyConfigDto> makeDefaultConfigLocked();
    oatpp::Object<TopologyConfigDto> cloneConfig(const oatpp::Object<TopologyConfigDto>& dto) const;
    void normalizeConfigLocked(const oatpp::Object<TopologyConfigDto>& dto);

private:
    mutable std::mutex mutex_;
    oatpp::Object<TopologyConfigDto> config_;
    bool loaded_;
    std::shared_ptr<oatpp::parser::json::mapping::ObjectMapper> objectMapper_;
};

#endif // TOPOLOGYCONFIGSERVICE_HPP
