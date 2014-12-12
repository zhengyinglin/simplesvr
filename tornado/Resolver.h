#ifndef _TORNADO_RESOLVER_H_
#define _TORNADO_RESOLVER_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <functional>
#include <vector>
#include "IOLoop.h"
#include "ares.h" //c-ares

/*
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>*/


namespace tornado
{

//https://github.com/gevent/gevent/blob/master/gevent/ares.pyx

class Resolver 
{
public:
    typedef std::vector< std::string > Hosts;
    typedef std::function<void(int, const Hosts* )> Callback;
    Resolver();
    virtual ~Resolver();
    
    int init(int flags=0, int tries=0, int ndots=0, int udp_port=0, int tcp_port=0);
    void gethostbyname(Callback cb, const char* name, int family=AF_INET, int timeout_ms = 1000);

private:
    static void sock_state_callback(void *data, int socket, int read, int write);
    static void ares_host_callback(void *arg, int status, int timeouts, struct hostent* host);
    void run_callback(int err, const Hosts* hosts);
    void sock_state_callback(int socket, int read, int write);
    void on_timer(tornado::IOLoop::TimerID tid, int64_t expiration);
    void process_fd(int32_t fd, uint32_t events);
    inline void remove_timer()
    {
        if(timer_)
        {
            io_loop_->removeTimeout(timer_);
            timer_ = 0;
        }
    }

private:
    ares_channel channel_;
    uint32_t state_;
    tornado::IOLoop*  io_loop_;
    tornado::IOLoop::TimerID  timer_;
    Callback   cb_;
};

}//namespace tornado
#endif //_TORNADO_RESOLVER_H_
  