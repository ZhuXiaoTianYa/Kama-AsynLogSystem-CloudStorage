#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>

#include "AsyncBuffer.hpp"

namespace mylog
{
    enum class AsyncType {ASYNC_SAFE,ASYNC_UNSAFE};
    using functor=std::function<void(Buffer&)>;
    class AsyncWorker
    {
    public:
    using ptr=std::shared_ptr<AsyncWorker>;
    AsyncWorker(const functor& cb,AsyncType async_type=AsyncType::ASYNC_SAFE)
    :callback_(cb),async_type_(async_type)
    {}

    private:
    AsyncType async_type_;
    std::atomic<bool> stop_;
    std::mutex mtx_;
    mylog::Buffer buffer_productor_;
    mylog::Buffer buffer_consumer_;
    
    functor callback_;
    };

}