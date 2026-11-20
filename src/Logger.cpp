#include "Logger.h"
#include <time.h>
#include <iostream>

// 设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_logLevel = level;
}

// 写日志，把日志信息写入lockqueue
void Logger::Log(std::string msg)
{
    m_lckQue.Push(msg);
}

// 获取类单例
Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    // 启动写日志线程
    std::thread writeLogTask([&]()
                             {
        while(1)
        {
            //获取当前的日期，取日志信息，写入对应的日志文件
            time_t now=time(nullptr);
            tm* nowtm=localtime(&now);

            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1990,nowtm->tm_mon+1,nowtm->tm_mday);

            FILE* pf=fopen(file_name,"a+");
            if(nullptr==pf)
            {
                std::cout<<"logger file "<<file_name<<" open error!"<<std::endl;
                exit(1);
            }
            std::string msg=m_lckQue.Pop();

            char time_buf[128]={0};
            sprintf(time_buf,"%d:%d:%d =>[%s] ",nowtm->tm_hour,nowtm->tm_min,nowtm->tm_sec,(m_logLevel==INFO?"info":"error"));

            msg.insert(0,time_buf);
            msg.append("\n");

            fputs(msg.c_str(),pf);
            fclose(pf);
        } });

    writeLogTask.detach();
}
