/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#ifndef _TORNADO_EPOLL_H_
#define _TORNADO_EPOLL_H_

#include <sys/epoll.h>

namespace tornado
{

class EPollImpl
{
public:
  EPollImpl(int events_num = 128);
  virtual ~EPollImpl();

  int registerFD(int32_t fd, uint32_t events);
  int modifyFD(int32_t fd, uint32_t events);
  int unregisterFD(int32_t fd);
  int poll(int timeoutMs, struct epoll_event*  &events);

private:
  int                  epollfd_;
  struct epoll_event*  events_;
  int                  events_num_;
};


}//namespace tornado

#endif  // _TORNADO_EPOLL_H_
