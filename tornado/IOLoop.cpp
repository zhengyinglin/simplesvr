#include "IOLoop.h"
#include "Logging.h"
#include "Util.h"
#include <signal.h>
#include <sys/signalfd.h>

namespace tornado
{

IOLoop::IOLoop(int maxfd /*= DEFAULT_MAX_FD*/):
        impl_(),
        handle_num_(0),
        handlers_(maxfd, STHander()),
        running_(false)
{
}

IOLoop::~IOLoop()
{
}

int IOLoop::addHandler(int32_t fd, HandlerCallback handler, uint32_t events)
{  
    if( fd < MIN_FD || fd >= (int)handlers_.size() )
    {
        LOG_ERROR("invalid fd=%d maxfd=%zu", fd, handlers_.size());  
        return -1;
    }

    assert( handlers_[fd].fd == -1 );

    ++handle_num_;

    handlers_[fd].fd = fd;
    handlers_[fd].cb = handler;

    int result = impl_.registerFD(fd, events | ERROR );
    if(result)
    {
        LOG_ERROR("fd=%d, events=%0x0x registerFD failed result=%d", fd, events, result);  
    }
    return result;
}


int IOLoop::updateHandler(int32_t fd, uint32_t events)
{
    if( fd < MIN_FD || fd >= (int)handlers_.size() )
    {
        LOG_ERROR("invalid fd=%d maxfd=%zu", fd, handlers_.size());  
        return -1;
    }

    int result = impl_.modifyFD(fd, events | ERROR );
    if(result)
    {
        LOG_ERROR("fd=%d, events=%0x0x modifyFD failed result=%d", fd, events, result);  
    }
    return result;
}


int IOLoop::removeHandler(int32_t fd)
{
    if( fd < MIN_FD || fd >= (int)handlers_.size() )
    {
        LOG_ERROR("invalid fd=%d maxfd=%zu", fd, handlers_.size());  
        return -1;
    }

    if( handlers_[fd].fd == -1)
    {
        LOG_ERROR("invalid fd=%d  ..do nothing", fd); 
        return 0;
    }
    --handle_num_;

    handlers_[fd].fd = -1;
    handlers_[fd].cb.clear();
    int result = impl_.unregisterFD(fd);
    if(result)
    {
        LOG_ERROR("fd=%d, unregisterFD failed result=%d", fd, result);  
    }    
    return result;
}


int32_t IOLoop::addSignalHandler(int signum, HandlerCallback handler)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, signum);
    // block watched signals: otherwise the default handler is called
    if(sigprocmask(SIG_BLOCK, &mask, 0)) 
    {
        LOG_ERROR("sigprocmask faild err=%s", STR_ERRNO);  
        return -1;
    }

    int32_t sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if(sfd < 0)
    {
        LOG_ERROR("signalfd faild err=%s", STR_ERRNO);  
        return sfd;
    }

    if( addHandler(sfd, handler, IOLoop::READ) )
    {
        LOG_ERROR("addHandler faild sfd=%d", sfd);  
        removeHandler(sfd);
        ::close(sfd);
        return -1;
    }
    return sfd;
}

int IOLoop::start()
{
    if(running_)
    {
        LOG_ERROR_STR("IOLoop is already running");
        return -1;
    }

    //self._setup_logging()
     running_ = true;
     uint32_t loopnum = 0;
     while(true)
     {
         LOG_DEBUG("IOLoop iterator(%u) start", ++loopnum);
         if(false == callbacks_.empty())
         {
             //copy 
             std::vector<Callback>  callbacks;
             callbacks.swap( callbacks_ );
             LOG_DEBUG("IOLoop run callbacks size = %zu", callbacks.size());
             for(auto iter = callbacks.begin(); iter != callbacks.end(); ++iter)
             {
                (*iter)();
             }
         }

         if(false == timeouts_.empty())
         { 
             timeouts_.runOnce( TimeUtil::curTimeMS() ); // noly can run one
         }

         //外部终止循环
         if(false == running_)
         {
             LOG_WARN_STR("running_ false exit loop");
             break;
         }
         
         //没有处理请求退出循环
         if( handle_num_ <= 0 && callbacks_.empty() && timeouts_.empty() )
         {
             LOG_WARN_STR("nothing to process exit loop");
             break;
         }

         int poll_timeout = 60 * 1000;//60s 
         int64_t now = TimeUtil::curTimeMS();
         if(false == callbacks_.empty())
         {
            poll_timeout = 0;
         }
         else if(timeouts_.nextExpiration() - now < poll_timeout )
         {
             if(timeouts_.nextExpiration() <= now)
                 poll_timeout = 0;
             else
                 poll_timeout =  timeouts_.nextExpiration() - now;
         }
         LOG_DEBUG("IOLoop poll_timeout = %d", poll_timeout);

         struct epoll_event*  event_list = NULL;
         int numEvents = impl_.poll(poll_timeout, event_list);
         for(int i=0; i<numEvents; i++)
         {
             int32_t fd = event_list[i].data.fd;
             uint32_t events = event_list[i].events;
             assert( fd >= MIN_FD && fd < (int)handlers_.size() );
             if( handlers_[fd].fd >= 0 )
             {
                 if(handlers_[fd].cb)
                 {
                     (handlers_[fd].cb)(fd, events);
                 }
                 else
                 {
                     LOG_WARN("handlers_[fd=%d].cb is null", fd );
                 }
             }
         }

     }//while(true)
     return 0;
}
               
int IOLoop::stop()
{
    running_ = false;
    //self._waker.wake()
    return 0;
}


IOLoop::TimerID  IOLoop::addTimeout(int delayMS, TimeCallback callback)
{
    return timeouts_.add(TimeUtil::curTimeMS(), delayMS, callback);
}

bool IOLoop::removeTimeout(TimerID  id)
{
   return timeouts_.erase(id); 
}


void IOLoop::addCallback(Callback callback)
{
    callbacks_.push_back(std::move(callback));
}


}//namespace tornado