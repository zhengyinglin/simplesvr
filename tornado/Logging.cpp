#include "Logging.h"

#include <sys/types.h>
#include <unistd.h>
#include <string>


#include <sys/stat.h>
#include <fcntl.h>

#include <string.h>
#include <libgen.h>
#include <stdarg.h>
#include <sys/time.h>


DEFINE_int32(loglevel, tornado::DEBUG, "DEBUG"); 
DEFINE_int32(logbufsecs, 10, "10 seconds");
DEFINE_string(logfile, "", "logfile name");
DEFINE_int32(force_flush, 0, "force_flush");

namespace tornado
{

int init_log(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    
    std::string logfile = FLAGS_logfile;
    if(logfile.empty())
    {
        long long int pid = getpid();
        logfile = std::to_string( pid ) + ".log";
    }
    return LogFileObject::instance().createLogfile(logfile.c_str());
}



bool LogFileObject::createLogfile(const char* filename)
{
    this->close();
        
    int fd = open(filename, O_WRONLY | O_CREAT | O_EXCL , 0664);
    if (fd == -1)
    {
        if(errno == EEXIST)
        {
            fd = open(filename, O_WRONLY | O_EXCL , 0664);
        }
        if(fd == -1)
        {
            printf("open %s fail errno = %d , %s\n", filename, errno, strerror(errno) ); 
            return false;
        }
    }
    fcntl(fd, F_SETFD, FD_CLOEXEC);
        
    file_ = fdopen(fd, "a");  // Make a FILE*.
    if (file_ == NULL)
    {  
        printf("fdopen fail errno = %d , %s\n", errno, strerror(errno) ); 
        ::close(fd);
        return false;
    }
    return true;
}

void LogFileObject::writeString(const char* levelname, const char* filename, const char* funname, int line, const char *pszMessage)
{
    if(!file_)
    {
        return ;
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t log_time = static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;

    static char buff[1024 * 100];
    int index = snprintf(buff, sizeof(buff), "%s|%u.%u|%s|%d|%s|%s", levelname, (uint32_t)tv.tv_sec, (uint32_t)tv.tv_usec/1000, filename, line, funname, pszMessage);

    buff[index] = '\n';

    bytes_since_flush_ += fwrite(buff, 1, index+1, file_);

    if( FLAGS_force_flush ||
            (bytes_since_flush_ >= 1000000) ||
            (log_time >= next_flush_time_) ) 
    {
        flush(log_time);
    }
}

void LogFileObject::write(const char* levelname, const char* filename, const char* funname, int line, const char *pszFormat, ...)
{
    if(!file_)
    {
        return ;
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t log_time = static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;

    static char buff[1024 * 100];
    int index = snprintf(buff, sizeof(buff), "%s|%u.%u|%s|%d|%s|", levelname, (uint32_t)tv.tv_sec, (uint32_t)tv.tv_usec/1000, filename, line, funname);


    va_list otherarg;
    va_start(otherarg, pszFormat);
    index += vsnprintf(buff+index, sizeof(buff)- index - 1, pszFormat, otherarg);
    va_end(otherarg);
    buff[index] = '\n';

    bytes_since_flush_ += fwrite(buff, 1, index+1, file_);

    if( FLAGS_force_flush ||
            (bytes_since_flush_ >= 1000000) ||
            (log_time >= next_flush_time_) ) 
    {
        flush(log_time);
    }
}
    
    
void LogFileObject::flush(int64_t cur_time)
{
    fflush(file_);
    bytes_since_flush_ = 0;
    const int64_t next = (FLAGS_logbufsecs  * static_cast<int64_t>(1000000));  // in usec
    next_flush_time_ = cur_time + next;
}

void LogFileObject::close()
{
    if(file_)
    {
        flush(0);
        fclose(file_);
        file_ = NULL;
    }
    bytes_since_flush_ = 0;
    next_flush_time_ = 0;
}

bool LogFileObject::shouldWrite(int level)
{
    return FLAGS_loglevel >= level;
}


} // namespace tornado
 

