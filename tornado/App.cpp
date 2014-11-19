#include "App.h"
#include <signal.h>
#include <assert.h>
#include "Logging.h"
#include "boost/bind.hpp"


namespace tornado
{


App::App():
    ioloop_( IOLoop::instance() ),
    running_(false),
    tid_(0)
{
}

App::~App()
{
}

void App::signal_handler(int32_t fd, uint32_t events)
{  
    TORNADO_LOG_WARN("Caught signal : will stop server signal fd=%d", fd);
    //must be remove 
    ioloop_->removeHandler(fd);
    stop();
}

void App::start()
{
    assert( !running_  );
    int32_t sfd = ioloop_->addSignalHandler(SIGTERM, 
        boost::bind(&App::signal_handler, this, _1, _2)  );
    if(sfd < 0)
    {
        TORNADO_LOG_ERROR_STR("addSignalHandler failed");
        return ;
    }
    TORNADO_LOG_INFO("addSignalHandler fd=%d", sfd);

    running_ = true;
    tid_ = ioloop_->addTimeout(0, 
        boost::bind(&App::runOneLoop, this, _1, _2) );

    ioloop_->start(); 
}

void App::stop()
{
    if(running_)
    {
        running_ = false;
        //remove anyway
        ioloop_->removeTimeout(tid_);
    }
}

void App::runOneLoop(IOLoop::TimerID  tid, int64_t expiration)
{
    TORNADO_LOG_INFO_STR("");

    if(running_ == false)
    {
        return ;
    }

    int delayMs = oneLoopProcessData();

    //run again
    tid_ = ioloop_->addTimeout(delayMs, 
        boost::bind(&App::runOneLoop, this, _1, _2) );
}


}//namespace tbusapp


