#include <unordered_map>
#include "AsyncLogger.hpp"

namespace mylog
{
    //通过单例对象对日志器进行管理，懒汉模式
    class LoggerManager
    {
    public:
        static LoggerManager& GetInstance()
        {
            static LoggerManager eton;
            return eton;
        }

        AsyncLogger::ptr GetLogger(const std::string &name)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            std::unordered_map<std::string,AsyncLogger::ptr>::iterator it=loggers_.find(name);
            if(it==loggers_.end())
                return AsyncLogger::ptr();
            return it->second;
        }

    private:
        LoggerManager()
        {
            std::unique_ptr<LoggerBuilder> builder(new LoggerBuilder());
            builder->BuildLoggerName("default");
            default_logger_=builder->Build();
            loggers_.insert(std::make_pair("default",default_logger_));
        }

    private:
        std::mutex mtx_;
        AsyncLogger::ptr default_logger_; //默认日志器
        std::unordered_map<std::string,AsyncLogger::ptr> loggers_;  //存放日志器
    };


}