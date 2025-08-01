#pragma once
#include <iostream>
#include <string>
#include <unordered_map>
#include <pthread.h>
#include "Config.hpp"
namespace storage
{

    typedef struct StorageInfo
    {
        time_t mtime_;
        time_t atime_;
        size_t fsize_;
        std::string storage_path_;
        std::string url_;

        bool NewStorageInfo(const std::string &storage_path)
        {
            FileUtil f(storage_path);
            if(!f.Exists())
            {
                return false;
            }
            mtime_=f.LastModifyTime();
            atime_=f.LastAccessTime();
            fsize_=f.FileSize();
            storage_path_=storage_path;
            storage::Config *config=storage::Config::GetInstance();
            url_=config->GetDownPrefix()+f.FileName();
            return true;
        }

    }StorageInfo;


    class DataManager
    {
    private:
        std::string storage_file_;
        pthread_rwlock_t rwlock_;
        std::unordered_map<std::string,StorageInfo> table_;
        bool need_persist;

    public:
        DataManager()
        {
            storage_file_=Config::GetInstance()->GetStorageInfo();
            pthread_rwlock_init(&rwlock_,NULL);
            need_persist=false;
            InitLoad();
            need_persist=true;
        }

        bool Storage()
        {
            std::vector<StorageInfo> arr;
            if(!GetAll(&arr))
            {
                return false;
            }
            Json::Value root;
            for(auto e:arr)
            {
                Json::Value item;
                item["mtime"]=(Json::Int64)e.mtime_;
                item["atime"]=(Json::Int64)e.atime_;
                item["fsize"]=(Json::Int64)e.fsize_;
                item["url"]=e.url_.c_str();
                item["storage_path"]=e.storage_path_.c_str();
                root.append(item);
            }

            std::string body;
            JsonUtil::Serialize(root,&body);
            FileUtil f(storage_file_);
            if(f.SetContent(body.c_str(),body.size())==false)
            {

            }
            return true;
        }

        bool GetOneByURL(const std::string &key,StorageInfo *info)
        {
            pthread_rwlock_rdlock(&rwlock_);
            if(table_.find(key)==table_.end())
            {
                pthread_rwlock_unlock(&rwlock_);
                return false;
            }
            *info = table_[key];
            pthread_rwlock_unlock(&rwlock_);
            return true;
        }

        bool Insert(const storage::StorageInfo& info)
        {
            pthread_rwlock_wrlock(&rwlock_);
            table_[info.url_]=info;
            pthread_rwlock_unlock(&rwlock_);
            if(need_persist==true&&Storage()==false)
            {
                return false;
            }
            return true;
        }

        bool GetAll(std::vector<StorageInfo> *arry)
        {
            pthread_rwlock_rdlock(&rwlock_);
            for(auto e: table_)
            {
                arry->emplace_back(e.second);
            }
            pthread_rwlock_unlock(&rwlock_);
            return true;
        }

        bool InitLoad()
        {
            storage::FileUtil f(storage_file_);
            if(!f.Exists())
            {
                return true;
            }

            std::string body;
            if(!f.GetContent(&body))
                return false;
            Json::Value root;
            storage::JsonUtil::UnSerialize(body,&root);
            for(int i=0;i<root.size();i++)
            {
                storage::StorageInfo info;
                info.fsize_=root[i]["fsize"].asInt();
                info.atime_=root[i]["atime"].asInt();
                info.mtime_=root[i]["mtime"].asInt();
                info.storage_path_=root[i]["storage_path"].asString();
                info.url_=root[i]["url"].asString();
                Insert(info);
            }
            return true;
        }


    };

}