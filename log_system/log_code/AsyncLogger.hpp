#pragma once
#include <cassert>
#include <mutex>
#include <string>
#include <vector>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <stdarg.h>
#include "LogFlush.hpp"
#include "Level.hpp"
#include "AsyncWorker.hpp"

namespace mylog
{
    class AsyncLogger
    {
    public:
        using ptr=std::shared_ptr<AsyncLogger>;
        AsyncLogger(const std::string &logger_name,std::vector<LogFlush::ptr> &flushs,AsyncType type)
        :logger_name_(logger_name),
        flushs_(flushs.begin(),flushs.end()),
        asyncworker(std::make_shared<AsyncWorker>(std::bind(RealFlush,this,std::placeholders::_1),type))
        {}

        void Debug(const std::string& file,size_t line, const std::string format,...)
        {
            va_list va;
            va_start(va,format);
            char *ret;
            int r=vasprintf(&ret,format.c_str(),va);
            if(r==-1)
                perror("vasprintf failed!!!:");
            va_end(va);
            
            serialize(LogLevel::value::DEBUG,file,line,ret);
            free(ret);
            ret ==nullptr;
        }

        void Info(const std::string &file,size_t line,const std::string format,...)
        {
            va_list va;
            va_start(va,format);
            char *ret;
            int r=vasprintf(&ret,format.c_str(),va);
            if(r==-1)
                perror("vasprintf failed!!!: ");
            va_end(va);
            serialize(LogLevel::value::INFO,file,line,ret);
            free(ret);
            ret=nullptr;
        }

        void Warn(const std::string &file,size_t line,const std::string format,...)
        {
            va_list va;
            va_start(va,format);
            char *ret;
            int r=vasprintf(&ret,format.c_str(),va);
            if(r==-1)
                perror("vasprintf failed!!!: ");
            va_end(va);
            serialize(LogLevel::value::WARN,file,line,ret);
            free(ret);
            ret==nullptr;
        }

        void Error(const std::string& file,size_t line,const std::string format,...)
        {
            va_list va;
            va_start(va,format);
            char *ret;
            int r=vasprintf(&ret,format.c_str(),va);
            if(r==-1)
                perror("vasprintf failed!!!: ");
            va_end(va);
            serialize(LogLevel::value::ERROR,file,line,ret);
            free(ret);
            ret=nullptr;
        }

        void Fatal(const std::string &file,size_t line ,const std::string format,...)   
        {
            va_list va;
            va_start(va,format);
            char *ret;
            int r=vasprintf(&ret,format.c_str(),va);
            if(r==-1)
                perror("vasprintf failed!!!: ");
            va_end(va);
            serialize(LogLevel::value::FATAL,file,line,ret);
            free(ret);
            ret=nullptr;
        }

    protected:
        void serialize(LogLevel::value level,const std::string &file,size_t line,char* ret)
        {

        }

        void RealFlush(Buffer& buffer)
        {

        }

    protected:
        std::mutex mtx_;
        std::string logger_name_;
        std::vector<mylog::LogFlush::ptr> flushs_; //输出到指定方向 \
        std::vector<LogFlush> flush_;不能使用logflush作为元素类型，logflush是纯虚类，不能实例化
        mylog::AsyncWorker::ptr asyncworker;
    };

    class LoggerBuilder
    {
    public:
        using ptr=std::shared_ptr<LoggerBuilder>;
        void BuildLoggerName(const std::string &name) {logger_name_=name;}
        
        AsyncLogger::ptr Build()
        {
            assert(logger_name_.empty()==false); //必须有日志器名称
            // 如果写日志方式没有指定，那么采用默认的标准输出
            if(flushs_.empty())
                flushs_.emplace_back(std::make_shared<StdoutFlush>());
            return std::make_shared<AsyncLogger>(logger_name_,flushs_,async_type_);
        }


    protected:
        std::string logger_name_ = "async_logger"; //日志器名称
        std::vector<mylog::LogFlush::ptr> flushs_; //写日志方式
        AsyncType async_type_ = AsyncType::ASYNC_SAFE; // 用于控制缓存区是否增长
    };


}