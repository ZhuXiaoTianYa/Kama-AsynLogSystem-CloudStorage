#pragma once
#include <string>
#include <mutex>
#include "jsoncpp/json/json.h"
#include "Util.hpp"


namespace storage
{
    const char *Config_File="Storage.conf";
    class Config
    {
    private:
        int server_port_;
        std::string server_ip_;
        std::string download_prefix_;
        std::string deep_storage_dir_;
        std::string low_storage_dir_;
        std::string storage_info_;
        int bundle_format_;

    private:
        static std::mutex mtx_;
        static Config* instance_;
        Config()
        {
            if(ReadConfig()==false)
            {
                return;
            }
        }

    public:
        bool ReadConfig()
        {
            storage::FileUtil fu(Config_File);
            std::string content;
            if(!fu.GetContent(&content))
            {
                return false;
            }
            Json::Value root;
            if(storage::JsonUtil::UnSerialize(content,&root)==false)
            {
                return false;
            }
            server_port_=root["server_port"].asInt();
            server_ip_=root["server_ip"].asString();
            download_prefix_=root["download_prefix"].asString();
            storage_info_=root["storage_info"].asString();
            deep_storage_dir_=root["deep_storage_dir"].asString();
            low_storage_dir_=root["low_storage_dir"].asString();
            bundle_format_=root["bundle_format"].asInt();
            return true;
        }

        std::string GetServerIp()
        {
            return server_ip_;
        }

        std::string GetDownPrefix()
        {
            return download_prefix_;
        }
        
        std::string GetLowStorageDir()
        {
            return low_storage_dir_;
        }

        std::string GetDeepStorageDir()
        {
            return deep_storage_dir_;
        }

        int GetBundleFormat()
        {
            return bundle_format_;
        }

        int GetServerPort()
        {
            return server_port_;
        }


        std::string GetStorageInfo()
        {
            return storage_info_;
        }

    public:
        static Config* GetInstance()
        {
            if(instance_==nullptr)
            {
                mtx_.lock();
                if(instance_==nullptr)
                {
                    instance_=new Config();
                }
                mtx_.unlock();
            }
            return instance_;
        } 

    };
    
    std::mutex Config::mtx_;
    Config *Config::instance_=nullptr;


}