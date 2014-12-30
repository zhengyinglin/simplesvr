/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/
#ifndef _TORNADO_IO_LOOP_H_
#define _TORNADO_IO_LOOP_H_
#include <vector>
#include "EPoll.h"
#include "TimeoutQueue.h"
#include "Util.h"

namespace tornado
{

//单例，不可复制
class IOLoop
{
public:
    static const int   DEFAULT_MAX_FD = 10000 + 4; //最大文件句柄数
    static const int   MIN_FD = 3; // 排除掉标准文件描述符0 1 2
    static const uint32_t  READ  = EPOLLIN;
    static const uint32_t  WRITE = EPOLLOUT;
    static const uint32_t  ERROR = EPOLLERR | EPOLLHUP;

    typedef folly::TimeoutQueue::Id  TimerID;
    typedef folly::TimeoutQueue::Callback  TimeCallback;
    typedef std::function<void(int32_t, uint32_t)> HandlerCallback;
    typedef std::function<void()> Callback;
    struct STHander
    {
        int32_t  fd;
        HandlerCallback  cb;
        STHander():
            fd(-1),
            cb(nullptr)
        {}
    };

private:
    IOLoop(int maxfd = DEFAULT_MAX_FD);
    // noncopyable
    IOLoop(const IOLoop&) = delete;
    IOLoop& operator=(const IOLoop&) = delete;

public: 
    ~IOLoop();

    static IOLoop* instance()
    {
        static IOLoop  instance_;
        return &instance_;
    }

    inline size_t getMaxFd() const {return handlers_.size();}

    int addHandler(int32_t fd, HandlerCallback&& handler, uint32_t events);
    
    int updateHandler(int32_t fd, uint32_t events);
   
    int removeHandler(int32_t fd);

    int32_t addSignalHandler(int signum, HandlerCallback&& handler);
   
    int start();
    int stop();

    inline TimerID  addTimeout(int delayMS, TimeCallback&& callback)
    {
        return timeouts_.add(TimeUtil::curTimeMS(), delayMS, callback);
    }

    inline bool removeTimeout(TimerID id)
    {
        return timeouts_.erase(id); 
    }

    inline void addCallback(Callback&& callback)
    {
        callbacks_.push_back(std::move(callback));
    }

    inline int getHandlerNum()const
    {
        return handle_num_;
    }

    inline bool idle() const 
    { 
        return callbacks_.empty() && timeouts_.empty();
    }
      
private:
    EPollImpl   impl_;
    int         handle_num_; //当前注册fd 数量
    //文件描述符递增特性，用数组而不是map
    std::vector<STHander>  handlers_;
    std::vector<Callback>  callbacks_;
    folly::TimeoutQueue    timeouts_;
    bool                   running_;
};




}//namespace tornado
#endif //_TORNADO_IO_LOOP_H_
