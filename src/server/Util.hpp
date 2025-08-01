#pragma once
#include "jsoncpp/json/json.h"
#include <iostream>
#include <string>
#include <experimental/filesystem>
#include <unistd.h>
#include <fstream>
#include <memory>
#include <sys/stat.h>
#include <assert.h>
#include "bundle.h"

namespace storage
{


    static unsigned char FromHex(const unsigned char &x)
    {
        unsigned char y;
        if(x>='A'&&x<='Z')
            y=x-'A'+10;
        else if(x>='a'&&x<='z')
            y=x-'a'+10;
        else if(x>='0'&&x<='9')
            y=x-'0';
        else
            assert(0);
        return y;
    }

    static std::string UrlDecode(const std::string &str)
    {
        std::string strTemp="";
        size_t length=str.length();
        for(size_t i =0;i<length;i++)
        {
            if(str[i]=='%')
            {
                assert(i+2<length);
                unsigned char high=FromHex(str[++i]);
                unsigned char low=FromHex(str[++i]);
                strTemp+=high*16+low;
            }
            else
                strTemp+=str[i];
        }
        return strTemp;
    }


    class FileUtil
    {
    private:
        std::string filename_;

    public:
        FileUtil(const std::string& filename):filename_(filename){}

    
        int64_t FileSize()
        {
            struct stat s;
            int ret=stat(filename_.c_str(),&s);
            if(ret==-1)
            {
                return -1;
            }
            return s.st_size;
        }

        time_t LastAccessTime()
        {
            struct stat s;
            auto ret=stat(filename_.c_str(),&s);
            if(ret==-1)
            {
                return -1;
            }
            return s.st_atime;
        }

        time_t LastModifyTime()
        {
            struct stat s;
            auto ret=stat(filename_.c_str(),&s);
            if(ret==-1)
            {
                return -1;
            }
            return s.st_mtime;
        }

        bool GetPosLen(std::string* content,size_t pos,size_t len)
        {
            if(pos+len>FileSize())
            {
                return false;
            }
            std::ifstream ifs;
            ifs.open(filename_,std::ios::binary);
            if(ifs.is_open()==false)
            {
                return false;
            }

            ifs.seekg(pos,std::ios::beg);
            content->resize(len);
            ifs.read(&(*content)[0],len);
            if(!ifs.good())
            {
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }

        bool SetContent(const char* content,size_t len)
        {
            std::ofstream ofs;
            ofs.open(filename_.c_str(),std::ios::binary);
            if(ofs.is_open()==false)
            {
                return false;
            }
            ofs.write(content,len);
            if(!ofs.good())
            {
                ofs.close();
            }
            ofs.close();
            return true;
        }

        bool Compress(const std::string &content,int format)
        {
            std::string packed=bundle::pack(format,content);
            if(packed.size()==0)
            {
                return false;
            }
            FileUtil f(filename_);
            if(f.SetContent(packed.c_str(),packed.size())==false)
            {
                return false;
            }
            return true;
        }

        bool UnCompress(std::string &download_path)
        {
            std::string body;
            if(this->GetContent(&body)==false)
            {
                return false;
            }
            std::string unpack=bundle::unpack(body);
            FileUtil fu(download_path);
            if(fu.SetContent(unpack.c_str(),unpack.size())==false)
            {
                return false;
            }
            return true;
        }

        bool Exists()
        {
            return std::experimental::filesystem::exists(filename_);
        }

        bool CreateDirectory()
        {
            if(Exists())
                return true;
            return std::experimental::filesystem::create_directories(filename_);
        }

        std::string FileName()
        {
            auto pos=filename_.find_last_of("/");
            if(pos==std::string::npos)
                return filename_;
            return filename_.substr(pos+1,std::string::npos);
        }

        bool GetContent(std::string *content)
        {
            return GetPosLen(content,0,FileSize());
        }

    };


    class JsonUtil
    {
    public:
        static bool UnSerialize(const std::string &str,Json::Value *val)
        {
            Json::CharReaderBuilder crb;
            std::unique_ptr<Json::CharReader> ucr(crb.newCharReader());
            std::string err;
            if(ucr->parse(str.c_str(),str.c_str()+str.size(),val,&err)==false)
            {
                return false;
            }
            return true;
        }

        static bool Serialize(const Json::Value &val,std::string *str)
        {
            Json::StreamWriterBuilder swb;
            std::unique_ptr<Json::StreamWriter> usw(swb.newStreamWriter());
            std::stringstream ss;
            if(usw->write(val,&ss)!=0)
            {
                return false;
            }
            *str=ss.str();
            return true;
        }
    };


}