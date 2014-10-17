/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#ifndef _TORNADO_SOCKET_UTIL_H_
#define _TORNADO_SOCKET_UTIL_H_
#include <netinet/tcp.h>
#include <sys/time.h> 
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <time.h>


namespace tornado
{

#define ERRNO_WOULDBLOCK  ( errno == EAGAIN || errno == EWOULDBLOCK )

#define ERRNO_CONNRESET  ( errno == ECONNRESET || errno == ECONNABORTED ||  errno == EPIPE || errno == ETIMEDOUT )

#define ERRNO_INPROGRESS ( errno == EINPROGRESS )

#define ERRNO_ECONNABORTED ( errno == ECONNABORTED )

#define STR_ERRNO    strerror( errno ) 

class TimeUtil
{
public:
    static double  curTime()
    {
        struct timeval t_start;
        gettimeofday(&t_start, NULL);
        return  t_start.tv_sec  + (double)t_start.tv_usec / 1000000; 
    }
    static int64_t  curTimeMS()
    {
        struct timeval t_start;
        gettimeofday(&t_start, NULL);
        return  (int64_t)t_start.tv_sec * 1000 + t_start.tv_usec / 1000; 
    }

    //不是线程安全的
    static char*  getGMTTimestamp()
    {
        time_t  now = time(NULL);
        //struct tm*  timeinfo = localtime(&now);
        struct tm*  timeinfo = gmtime(&now);
        static char buff[128];
        size_t end = strftime(buff, 127, "%a, %d %b %Y %T %Z", timeinfo);
        buff[end] = 0;
        return buff;
    }       

};

class SocketUtil
{
public:
    static int setTcpnodealy(int32_t fd, int32_t val = 1 )
    {
		return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&val, (int32_t)sizeof(int32_t));
	}

	/*int SocketTool::set_sndbufsize(int32_t fd, int32_t sz ){
		return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&sz, (int32_t)sizeof(int32_t));
	}

	int SocketTool::set_rcvbufsize(int32_t fd, int32_t sz){
		return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&sz, (int32_t)sizeof(int32_t));
	}*/
        
	static int setKeepalive(int32_t fd, int32_t val)
    {
		return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*)&val, int(sizeof(int32_t)));
    }
	
	static int setReuseaddr(int32_t fd, int32_t val = 1)
    {
		return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&val, int(sizeof(int32_t)));
    }

    static int setNonblock(int32_t fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        flags |= (O_NONBLOCK);
        return fcntl(fd, F_SETFL, flags);
    }

    static int getSocketError(int fd)
    {
      int optval;
      socklen_t optlen = static_cast<socklen_t>(sizeof optval);

      if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
      {
          return errno;
      }
      return optval;
    }
};






} //namespace tornado
#endif //_TORNADO_SOCKET_UTIL_H_