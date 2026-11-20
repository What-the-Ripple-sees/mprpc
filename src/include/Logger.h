#pragma once

#include "LockQueue.h"

enum LogLevel
{
    INFO,
    ERROR,
};

// mprpc的日志系统
class Logger
{
private:
    int m_logLevel; // 记录日志级别
    LockQueue<std::string> m_lckQue;//日志缓冲队列

    //单例
    Logger();
    Logger(const Logger&)=delete;
    Logger(Logger&& )=delete;
public:
    //设置日志级别
    void SetLogLevel(LogLevel level);

    //写日志
    void Log(std::string msg);

    //获取类单例
    static Logger& GetInstance();
};

//定义日志宏
#define LOG_INFO(logmsgformat,...)\
do\
{\
    Logger& logger=Logger::GetInstance();\
    logger.SetLogLevel(INFO);\
    char c[1024]={0};\
    snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
    logger.Log(c);\
} while (0)

#define LOG_ERR(logmsgformat,...)\
do\
{\
    Logger& logger=Logger::GetInstance();\
    logger.SetLogLevel(ERROR);\
    char c[1024]={0};\
    snprintf(c,1024,logmsgformat,##__VA_ARGS__);\
    logger.Log(c);\
} while (0)
