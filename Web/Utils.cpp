
#include "Utils.hpp"

#include <fstream>
#include <cstring>
#include <cstdarg>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <ctime>

#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <sstream>

oatpp::String StaticFilesManager::getExtension(const oatpp::String& filename) {
  v_int32 dotPos = 0;
  for(v_int32 i = filename->size() - 1; i > 0; i--) {
    if(filename->data()[i] == '.') {
      dotPos = i;
      break;
    }
  }
  if(dotPos != 0 && dotPos < filename->size() - 1) {
    return oatpp::String((const char*)&filename->data()[dotPos + 1], filename->size() - dotPos - 1);
  }
  return nullptr;
}

oatpp::String StaticFilesManager::getFile(const oatpp::String& path) {
  if(!path) {
    return nullptr;
  }
  std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  auto& file = m_cache [path];
  if(file) {
    return file;
  }
  oatpp::String filename = m_basePath + "/" + path;
  file = oatpp::String::loadFromFile(filename->c_str());
  return file;
}

oatpp::String StaticFilesManager::guessMimeType(const oatpp::String& filename) {
  auto extension = getExtension(filename);
  if(extension) {
    
    if(extension=="html") {
      return "text/html";
    } else if(extension=="js"){
      return "application/javascript";
    } else if(extension == "m3u8"){
      return "application/x-mpegURL";
    } else if(extension == "mp4"){
      return "video/mp4";
    } else if(extension == "ts"){
      return "video/MP2T";
    } else if(extension == "mp3"){
      return "audio/mp3";
    }
    
  }
  return nullptr;
}

oatpp::String formatText(const char* text, ...) {
  char buffer[4097];
  va_list args;
  va_start (args, text);
  vsnprintf(buffer, 4096, text, args);
  va_end(args);
  return oatpp::String(buffer);
}

v_int64 getMillisTickCount(){
  std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>
  (std::chrono::system_clock::now().time_since_epoch());
  return millis.count();
}

std::string getCurrentTimeString() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");  // 精确到秒

    return oss.str();
}


std::string execWithOutput(const std::string& cmd) {
    std::array<char, 256> buffer;
    std::stringstream result;

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        throw std::runtime_error("popen() 失败");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result << buffer.data();
    }

    int retCode = pclose(pipe);
    if (retCode != 0) {
        result << "\n[编译失败，返回码: " << retCode << "]";
    }

    return result.str();
}

std::string percentDecode(const oatpp::String& s) {
  std::string out;
  out.reserve(s ? s->size() : 0);
  for (size_t i = 0; s && i < s->size(); ++i) {
    unsigned char c = static_cast<unsigned char>((*s)[i]);
    if (c == '%') {
      if (i + 2 < s->size() &&
          std::isxdigit(static_cast<unsigned char>((*s)[i+1])) &&
          std::isxdigit(static_cast<unsigned char>((*s)[i+2]))) {
        auto hex = std::string{s->c_str() + i + 1, 2};
        char decoded = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
        out.push_back(decoded);
        i += 2;
      } else {
        // 非法的 % 序列，原样保留或直接报错都行；这里选择原样保留
        out.push_back(c);
      }
    } else if (c == '+') {
      out.push_back(' ');
    } else {
      out.push_back(static_cast<char>(c));
    }
  }
  return out;
}