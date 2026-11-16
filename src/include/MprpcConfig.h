#pragma once

#include <unordered_map>
#include <string>

//框架读取配置文件
class MprpcConfig
{
private:
    void Trim(std::string& src_buf);

    std::unordered_map<std::string,std::string> m_configMap;
public:
    //负责加载解析文件
    void LoadConfigFile(const char* config_file);

    //查询配置信息
    std::string Load(const std::string &key);
};

