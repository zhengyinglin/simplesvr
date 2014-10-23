/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-22
*/

#ifndef _TORNADO_APP_H_
#define _TORNADO_APP_H_

#include "IOLoop.h"

namespace tornado
{

class App
{
public:
    App();
    virtual ~App();

    virtual void start();
    virtual void stop();

    void signal_handler(int32_t fd, uint32_t events);

    //该函数返回下一次调用的时间间隔  毫秒MS
    virtual int oneLoopProcessData() = 0;

protected:
    void runOneLoop(IOLoop::TimerID  tid, int64_t expiration);
    IOLoop*           ioloop_;
    bool              running_;
    IOLoop::TimerID   tid_;
};


}//namespace tornado

#endif  // _TORNADO_APP_H_

