#pragma once
#include <event.h>
#include <evhttp.h>
#include <event2/http.h>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <iomanip>
#include <regex>
#include "base64.h"
#include "DataManager.hpp"

extern storage::DataManager *data_;

namespace storage
{

    class Service
    {
    public:
        Service()
        {
            server_ip_=Config::GetInstance()->GetServerIp();
            server_port_=Config::GetInstance()->GetServerPort();
            download_prefix_=Config::GetInstance()->GetDownPrefix();
        }

        bool RunModule()
        {
            event_base *base=event_base_new();
            if(base==NULL)
            {
                return false;
            }

            sockaddr_in sin;
            memset(&sin,0,sizeof(sin));
            sin.sin_family=AF_INET;
            sin.sin_addr.s_addr=INADDR_ANY;
            sin.sin_port=htons(server_port_);
            evhttp *httpd=evhttp_new(base);
            if(evhttp_bind_socket(httpd,"0.0.0.0",server_port_)!=0)
            {
                return false;
            }
            evhttp_set_gencb(httpd,GenHandler,NULL);
            if(base)
            {
                if(event_base_dispatch(base)==-1)
                {

                }
            }
            if(base)
                event_base_free(base);
            if(httpd)
                evhttp_free(httpd);
            return true;
        }

        static void GenHandler(evhttp_request* req,void* arg)
        {
            std::string path=evhttp_uri_get_path(evhttp_request_get_evhttp_uri(req));
            path=storage::UrlDecode(path);
            if(path.find("/download/")!=std::string::npos)
            {
                printf("1\n");
                Download(req,arg);
            }
            else if(path.find("/upload")!=std::string::npos)
            {
                Upload(req,arg);
            }
            else if(path.find("/")!=std::string::npos)
            {
                ListShow(req,arg);
            }
            else
            {
                evhttp_send_reply(req,HTTP_NOTFOUND,"Not Found",NULL);
            }
        }

        static std::string TimetoStr(time_t t)
        {
            std::string tmp=std::ctime(&t);
            return tmp;
        }

        static std::string generateModernFileList(const std::vector<StorageInfo> &files)
        {
            std::stringstream ss;
            ss<<"<div class='file-list'><h3>已上传文件</h3>";
            for(const auto &file:files)
            {
                std::string Filename=FileUtil(file.storage_path_).FileName();
                std::string storage_type="low";
                if(file.storage_path_.find("deep")!=std::string::npos)
                {
                    storage_type="deep";
                }
                ss << "<div class='file-item'>"
                << "<div class='file-info'>"
                << "<span>📄" << Filename << "</span>"
                << "<span class='file-type'>"
                << (storage_type == "deep" ? "深度存储" : "普通存储")
                << "</span>"
                << "<span>" << formatSize(file.fsize_) << "</span>"
                << "<span>" << TimetoStr(file.mtime_) << "</span>"
                << "</div>"
                << "<button onclick=\"window.location='" << file.url_ << "'\">⬇️ 下载</button>"
                << "</div>";
            }
            ss << "</div>";
            return ss.str();
        }

        static std::string GetETag(const StorageInfo &info)
        {
            FileUtil fu(info.storage_path_);
            std::string etag=fu.FileName();
            etag+="-";
            etag+=std::to_string(info.fsize_);
            etag+="-";
            etag+=std::to_string(info.mtime_);
            return etag;
        }

        static std::string formatSize(uint64_t bytes)
        {
            const char *units[]={"B","KB","MB","GB"};
            int unit_index=0;
            double size=bytes;
            while(size>=1024&&unit_index<3)
            {
                size/=1024;
                unit_index++;
            }
            std::stringstream ss;
            ss<<std::fixed<<std::setprecision(2)<<size<<" "<<units[unit_index];
            return ss.str();
        }

        static void Upload(evhttp_request* req ,void* arg)
        {
            struct evbuffer* buf=evhttp_request_get_input_buffer(req);
            if(buf==nullptr)
            {
                return;
            }
            size_t len=evbuffer_get_length(buf);
            if(len==0)
            {
                evhttp_send_reply(req,HTTP_BADREQUEST,"file empty",NULL);
                return;
            }
            std::string content(len,0);
            if(-1==evbuffer_copyout(buf,(void*)content.c_str(),len))
            {
                evhttp_send_reply(req,HTTP_INTERNAL,NULL,NULL);
                return;
            }
            std::string filename=evhttp_find_header(req->input_headers,"FileName");
            filename=base64_decode(filename);
            std::string storage_type=evhttp_find_header(req->input_headers,"StorageType");
            std::string storage_path;
            if(storage_type=="low")
            {
                storage_path=Config::GetInstance()->GetLowStorageDir();
            }
            else if(storage_type=="deep")
            {
                storage_path=Config::GetInstance()->GetDeepStorageDir();
            }
            else
            {
                evhttp_send_reply(req,HTTP_BADREQUEST,"Illegal storage type",NULL);
                return;
            }
            FileUtil dirCreate(storage_path);
            dirCreate.CreateDirectory();
            storage_path+=filename;

            FileUtil fu(storage_path);
            if(storage_path.find("low_storage")!=std::string::npos)
            {
                if(false==fu.SetContent(content.c_str(),content.size()))
                {
                    evhttp_send_reply(req,HTTP_INTERNAL,"server error",NULL);
                    return;
                }
                else{
                    
                }
            }
            else
            {
                if(false==fu.Compress(content,Config::GetInstance()->GetBundleFormat()))
                {
                    evhttp_send_reply(req,HTTP_INTERNAL,"server error",NULL);
                }
                else
                {

                }
            }
            StorageInfo info;
            info.NewStorageInfo(storage_path);
            data_->Insert(info);
            evhttp_send_reply(req,HTTP_OK,"Success",NULL);
        }

        static void Download(evhttp_request* req,void* arg)
        {
            StorageInfo info;
            std::string resource_path=evhttp_uri_get_path(evhttp_request_get_evhttp_uri(req));
            resource_path=UrlDecode(resource_path);
            data_->GetOneByURL(resource_path,&info);
            std::string download_path=info.storage_path_;
            if(download_path.find(Config::GetInstance()->GetLowStorageDir())==std::string::npos)
            {
                FileUtil fu(info.storage_path_);
                download_path=Config::GetInstance()->GetLowStorageDir()+std::string(download_path.begin()+download_path.find_last_of("/")+1,download_path.end());
                FileUtil dirCreate(Config::GetInstance()->GetLowStorageDir());
                dirCreate.CreateDirectory();
                fu.UnCompress(download_path);
            }
            FileUtil fu(download_path);
            if(fu.Exists()==false&&info.storage_path_.find("deep_storage")!=std::string::npos)
            {
                evhttp_send_reply(req,HTTP_INTERNAL,NULL,NULL);
            }
            else if((fu.Exists()==false) && (info.storage_path_.find("low_storage")==std::string::npos))
            {
                evhttp_send_reply(req,HTTP_BADREQUEST,"file not exists",NULL);
            }

            bool retrans=false;
            std::string old_etag;
            auto if_range=evhttp_find_header(req->input_headers,"If-Range");
            if(if_range!=NULL)
            {
                old_etag==if_range;
                if(old_etag==GetETag(info))
                {
                    retrans=true;
                }
            }
            if(fu.Exists()==false)
            {
                evhttp_send_reply(req,404,download_path.c_str(),NULL);
                return;
            }
            evbuffer *outbuf=evhttp_request_get_output_buffer(req);
            int fd =open(download_path.c_str(),O_RDONLY);
            if(fd==-1)
            {
                evhttp_send_reply(req,HTTP_INTERNAL,strerror(errno),NULL);
                return;
            }
            if(-1==evbuffer_add_file(outbuf,fd,0,fu.FileSize()))
            {
            }
            evhttp_add_header(req->output_headers,"Accept-Ranges","bytes");
            evhttp_add_header(req->output_headers,"ETag",GetETag(info).c_str());
            evhttp_add_header(req->output_headers,"Content-Type","application/octet-stream");
            
            if(retrans==false)
            {
                evhttp_send_reply(req,HTTP_OK,"Success",NULL);
            }
            else
            {
                evhttp_send_reply(req,206,"breakpoint continuous transmission",NULL);
            }
            if(download_path!=info.storage_path_)
            {
                remove(download_path.c_str());
            }
        }

        static void ListShow(evhttp_request* req,void* arg)
        {
            std::vector<storage::StorageInfo> arry;
            data_->GetAll(&arry);
            std::ifstream templateFile("index.html");
            std::string templateContent((std::istreambuf_iterator<char>(templateFile)),std::istreambuf_iterator<char>());
            templateContent=std::regex_replace(templateContent,std::regex("\\{\\{FILE_LIST\\}\\}"),generateModernFileList(arry));
            templateContent=std::regex_replace(templateContent,std::regex("\\{\\{BACKEND_URL\\}\\}"),"http://"+storage::Config::GetInstance()->GetServerIp()+":"+std::to_string(storage::Config::GetInstance()->GetServerPort()));
            struct evbuffer *buf=evhttp_request_get_output_buffer(req);
            auto response_body=templateContent;
            evbuffer_add(buf,(const void*)response_body.c_str(),response_body.size());
            evhttp_add_header(req->output_headers,"Content-Type","text/html;charset=utf-8");
            evhttp_send_reply(req,HTTP_OK,NULL,NULL);
        }

    private:
        uint16_t server_port_;
        std::string server_ip_;
        std::string download_prefix_;
    };

}