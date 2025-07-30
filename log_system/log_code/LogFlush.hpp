#include <cassert>
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <unistd.h>
#include <functional>


namespace mylog
{
    class LogFlush
    {
    public:
        using ptr=std::shared_ptr<LogFlush>;
        virtual ~LogFlush() {}
        virtual void Flush(const char *data,size_t len)=0; //不同的写文件方式Flush的实现不同
    };

    class StdoutFlush: public LogFlush
    {
    public:
        using ptr=std::shared_ptr<StdoutFlush>;
        void Flush(const char *data,size_t len) override
        {
            std::cout.write(data,len);
        }
    };

    class FileFlush: public LogFlush
    {
    public:
        using ptr=std::shared_ptr<FileFlush>;
        FileFlush(const std::string &filename):filename_(filename)
        {
            //创建所给目录

            //打开文件
            fs_=fopen(filename_.c_str(),"ab");
            if(fs_==NULL)
            {
                std::cout<<__FILE__<<__LINE__<<"open log file failed"<<std::endl;
                perror(NULL);
            }
        }

        void Flush(const char *data,size_t len) override
        {
            fwrite(data,1,len,fs_);
            if(ferror(fs_))
            {
                std::cout<<__FILE__<<__LINE__<<"write log file failed"<<std::endl;
                perror(NULL);
            }

            //TODO
            
        }

    private:
        std::string filename_;
        FILE* fs_=NULL;
    };


    class RollFileFlush:public LogFlush
    {
        
    };

}