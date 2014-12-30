/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#ifndef _TORNADO_LOGGING_H_
#define _TORNADO_LOGGING_H_


#include "gflags/gflags.h"

namespace tornado
{

int init_log(int argc, char* argv[]);

//日志级别
enum LOG_LEVEL
{
    INFO  = 1,
    ERROR = 2,
    WARN  = 3,
    DEBUG = 4,
};

class LogFileObject 
{
public:
    ~LogFileObject()
    {
        this->close();
    }

    bool createLogfile(const char* filename);
    
    void writeString(const char* levelname, const char* filename, const char* funname, int line, 
        const char *pszMessage);

    void write(const char* levelname, const char* filename, const char* funname, int line, 
        const char *pszFormat, ...)  __attribute__((format(printf,6,7)));
  
    void flush(int64_t cur_time);
   
    bool shouldWrite(int level);

    void close();
    static LogFileObject& instance()
    {
        static LogFileObject _instance;
        return _instance;
    }
private:
     LogFileObject():
        file_(NULL),
        bytes_since_flush_(0),
        next_flush_time_(0)
    {}

private:
    FILE* file_;
    uint32_t bytes_since_flush_;
    int64_t next_flush_time_;
};

} // namespace tornado

//几个日志宏

#define TORNADO_LOG_INFO(szFormat, args...) \
        if(tornado::LogFileObject::instance().shouldWrite(tornado::INFO)) \
        { \
            tornado::LogFileObject::instance().write("INFO", __FILE__, __FUNCTION__,  __LINE__, szFormat, ##args); \
        };

#define TORNADO_LOG_ERROR(szFormat, args...) \
        if(tornado::LogFileObject::instance().shouldWrite(tornado::ERROR)) \
        { \
           tornado::LogFileObject::instance().write("ERROR", __FILE__, __FUNCTION__,  __LINE__, szFormat, ##args); \
        };


#define TORNADO_LOG_WARN(szFormat, args...) \
        if(tornado::LogFileObject::instance().shouldWrite(tornado::WARN)) \
        { \
           tornado::LogFileObject::instance().write("WARN", __FILE__, __FUNCTION__,  __LINE__, szFormat, ##args); \
        };

#define TORNADO_LOG_DEBUG(szFormat, args...) \
        if(tornado::LogFileObject::instance().shouldWrite(tornado::DEBUG)) \
        { \
           tornado::LogFileObject::instance().write("DEBUG", __FILE__, __FUNCTION__,  __LINE__, szFormat, ##args); \
        };



#define TORNADO_LOG_INFO_STR(message) \
        if(tornado::LogFileObject::instance().shouldWrite(tornado::INFO)) \
        { \
            tornado::LogFileObject::instance().writeString("INFO", __FILE__, __FUNCTION__,  __LINE__, message); \
        };

#define TORNADO_LOG_ERROR_STR(message) \
        if(tornado::LogFileObject::instance().shouldWrite(tornado::ERROR)) \
        { \
           tornado::LogFileObject::instance().writeString("ERROR", __FILE__, __FUNCTION__,  __LINE__, message); \
        };


#define TORNADO_LOG_WARN_STR(message) \
        if(tornado::LogFileObject::instance().shouldWrite(tornado::WARN)) \
        { \
           tornado::LogFileObject::instance().writeString("WARN", __FILE__, __FUNCTION__,  __LINE__, message); \
        };

#define TORNADO_LOG_DEBUG_STR(message) \
        if(tornado::LogFileObject::instance().shouldWrite(tornado::DEBUG)) \
        { \
           tornado::LogFileObject::instance().writeString("DEBUG", __FILE__, __FUNCTION__,  __LINE__, message); \
        };


#endif //_TORNADO_LOGGING_H_