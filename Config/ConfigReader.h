#pragma once
#include <string>
#include <map>

class ConfigReader {
public:
    static std::string getStr(const std::string& key, const std::string& default_value = "");
    static int getInt(const std::string& key, int default_value = 0);
    static bool getBool(const std::string& key, bool default_value = false);

private:
    ConfigReader(); // 构造函数私有化
    bool load(const std::string& path);
    static ConfigReader& instance();

    std::map<std::string, std::string> config_;
    bool loaded_;
};
