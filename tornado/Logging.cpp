#include "Logging.h"
#include <string>


namespace tornado
{

Logger::Logger():
    level_(DEBUG)
{}


Logger& Logger::instance()
{
    static Logger _instance;
    return _instance;
}

void Logger::setLevel(LEVEL level)
{
    level_ = level;
}

Logger::LEVEL Logger::getLevel()
{
    return level_;
}


void set_log_level(int argc, char** args)
{
    if(argc > 1)
    { 
       Logger::LEVEL level = Logger::DEBUG;
       std::string levelname(args[1]);
       if(levelname == "None" || levelname == "none")
       {
           level = Logger::None;
       }
       else if(levelname == "ERROR" || levelname == "error")
       {
           level = Logger::ERROR;
       }
       else if(levelname == "WARNING" || levelname == "warning" || levelname == "WARN" || levelname == "warn")
       {
           level = Logger::WARNING;
       }
       else if(levelname == "INFO" || levelname == "info")
       {
           level = Logger::INFO;
       }
       Logger::instance().setLevel( level );
    }
    const char* levelname[] = {"", "ERROR", "WARNING", "INFO", "DEBUG"};
    LOG_INFO("current log level=%s", levelname[Logger::instance().getLevel()] );
}      

} // namespace tornado
 