#include "Service.hpp"
#include <thread>
#include <unistd.h>

storage::DataManager *data_;

void service_module()
{
    storage::Service service;
    service.RunModule();
}

void CreateDaemon()
{
    pid_t pid=fork();
    if(pid<0)
    {
        std::cerr<<"Fork failed!"<<std::endl;
        exit(1);
    }
    if(pid>0)
    {
        exit(0);
    }
    if(setsid()<0)
    {
        std::cerr<<"Failed to create session!"<<std::endl;
        exit(1);
    }
    data_=new storage::DataManager();
    std::thread t1(service_module);
    t1.join();
    exit(0);
}

int main()
{
    CreateDaemon();
    //delete(tp);
    return 0;
}