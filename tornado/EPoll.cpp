#include "EPoll.h"
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <strings.h>

namespace tornado
{

EPollImpl::EPollImpl(int events_num /*= 128*/):
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_num_(events_num)
{
    assert( epollfd_ >= 0 );
    assert( events_num_ > 0 && events_num_ < 1024 * 100 );
    events_ = new struct epoll_event [events_num_];
}

EPollImpl::~EPollImpl()
{
    if(epollfd_ >= 0)
    {
        ::close(epollfd_);
    }
    
    delete []events_;
}


int EPollImpl::registerFD(int32_t fd, uint32_t events)
{
   struct epoll_event ep_event;
   bzero(&ep_event, sizeof(ep_event));
   
   ep_event.data.fd = fd;
   ep_event.events = events;

   return ::epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ep_event);
}


int EPollImpl::modifyFD(int32_t fd, uint32_t events)
{
   struct epoll_event ep_event;
   bzero(&ep_event, sizeof(ep_event));
   
   ep_event.data.fd = fd;
   ep_event.events = events;

   return ::epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ep_event);
}

int EPollImpl::unregisterFD(int32_t fd)
{
   struct epoll_event ep_event;
   bzero(&ep_event, sizeof(ep_event));
   
   ep_event.data.fd = fd;

   return ::epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, &ep_event);
}


int EPollImpl::poll(int timeoutMs, struct epoll_event*  &events)
{
    events = NULL;
    int numEvents = epoll_wait(epollfd_, events_, events_num_, timeoutMs);
    if (numEvents == 0)
    {
        return 0;
    }
    if (numEvents < 0)
    {
        if (errno == EINTR)
        {
            return 0;
        }
        return -1;
    }
    events = events_;
    return numEvents;
}


}//namespace tornado

