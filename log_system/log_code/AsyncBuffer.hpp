// 日志缓冲区类设计
#pragma once
#include <cassert>
#include <string>
#include <vector>

namespace mylog
{

    class Buffer
    {

    protected:
        std::vector<char> buffer_; //缓存区
        size_t write_pos_; //生产者此时的位置
        size_t read_pos_; //消费者此时的位置
    };

}
