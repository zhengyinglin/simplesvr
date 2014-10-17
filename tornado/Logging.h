/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/
#ifndef _TORNADO_LOGGING_H_
#define _TORNADO_LOGGING_H_

#include <stdio.h>
#include <stdint.h>
#include "Util.h"

namespace tornado
{


#define LOG_DEBUG(szFormat, args...)  \
    if(tornado::Logger::instance().getLevel() >= tornado::Logger::DEBUG) { \
        printf( "DEBUG|%.3lf|%s|%d|%s|"#szFormat "\n", tornado::TimeUtil::curTime(), __FILE__, __LINE__, __func__, ##args); \
    }


#define LOG_INFO(szFormat, args...)  \
    if(tornado::Logger::instance().getLevel() >= tornado::Logger::INFO) { \
        printf( "INFO|%.3lf|%s|%d|%s|"#szFormat "\n", tornado::TimeUtil::curTime(), __FILE__, __LINE__, __func__, ##args); \
    }


#define LOG_WARN(szFormat, args...)  \
    if(tornado::Logger::instance().getLevel() >= tornado::Logger::WARNING) { \
        printf( "WARN|%.3lf|%s|%d|%s|"#szFormat "\n", tornado::TimeUtil::curTime(), __FILE__, __LINE__, __func__, ##args); \
    }


#define LOG_ERROR(szFormat, args...)  \
    if(tornado::Logger::instance().getLevel() >= tornado::Logger::ERROR) { \
        printf( "ERROR|%.3lf|%s|%d|%s|"#szFormat "\n", tornado::TimeUtil::curTime(), __FILE__, __LINE__, __func__, ##args); \
    }



class Logger
{
public:
    enum LEVEL{
        None = 0,
        ERROR = 1,
        WARNING = 2,
        INFO = 3,
        DEBUG = 4
    };
private:
    Logger();
public:
    static Logger& instance();
    void setLevel(LEVEL level);
    LEVEL getLevel();

private:
    LEVEL  level_;
};


void set_log_level(int argc, char** args);
       

} // namespace tornado
#endif //_TORNADO_LOGGING_H_