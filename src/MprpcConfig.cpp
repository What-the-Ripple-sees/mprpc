#include "MprpcConfig.h"
#include <iostream>
#include <string>

// 负责加载解析文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf)
    {
        std::cout << config_file << " is not exist!" << std::endl;
        exit(1);
    }

    // 读取文件内容：忽略注释，检查配置项是否正确，去掉每行多余的空格...
    while (!feof(pf))
    {
        char buf[512] = {0};
        fgets(buf, 512, pf);

        // 去空格
        std::string src_buf(buf);
        Trim(src_buf);

        if (src_buf.empty()||src_buf[0] == '#' )
        {
            continue;
        }

        // 解析配置项
        int idx = src_buf.find('=');
        if (idx == -1)
            continue;
        std::string key, value;
        key = src_buf.substr(0, idx);
        Trim(key);
        int endidx=src_buf.find('\n',idx);
        value = src_buf.substr(idx + 1, endidx - idx-1 );
        Trim(value);
        m_configMap.insert({key, value});
    }
}

// 查询配置信息
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    else
        return it->second;
}

void MprpcConfig::Trim(std::string &src_buf)
{
    int idx = src_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        src_buf = src_buf.substr(idx, src_buf.size() - idx);
    }
    idx = src_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        src_buf = src_buf.substr(0, idx + 1);
    }
}