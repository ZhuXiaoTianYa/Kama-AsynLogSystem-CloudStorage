#pragma once
#include <string>

namespace mylog
{
    class LogLevel
    {
    public:
        enum class value{ DEBUG,INFO,WARN,ERROR,FATAL };
    };
}