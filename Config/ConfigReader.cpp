#include "ConfigReader.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <algorithm>

ConfigReader::ConfigReader() : loaded_(false) {
    // 从环境变量获取配置路径
    const char* env_path = std::getenv("QKDSIM_CONFIG_PATH");
    std::string path = env_path ? env_path : "../Config/config.txt";
    loaded_ = load(path);
}

bool ConfigReader::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    std::string line;
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        key.erase(0, key.find_first_not_of(" \t\r\n"));
        key.erase(key.find_last_not_of(" \t\r\n") + 1);
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        config_[key] = value;
    }

    return true;
}

ConfigReader& ConfigReader::instance() {
    static ConfigReader instance;
    return instance;
}

std::string ConfigReader::getStr(const std::string& key, const std::string& default_value) {
    auto& cfg = instance();
    auto it = cfg.config_.find(key);
    return it != cfg.config_.end() ? it->second : default_value;
}

 void ConfigReader::setStr(const std::string& key,const std::string& value){
    auto& cfg = instance();
    cfg.config_[key]=value;
 }

int ConfigReader::getInt(const std::string& key, int default_value) {
    std::string value = getStr(key);
    try {
        return std::stoi(value);
    } catch (...) {
        return default_value;
    }
}

bool ConfigReader::getBool(const std::string& key, bool default_value) {
    std::string value = getStr(key);
    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (value == "true" || value == "1" || value == "yes" || value == "on") return true;
    if (value == "false" || value == "0" || value == "no" || value == "off") return false;
    return default_value;
}
