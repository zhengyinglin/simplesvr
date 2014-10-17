#include "TcpServer.h"
#include "IOLoop.h"
#include "Util.h"
#include "Logging.h"
#include "boost/bind.hpp"

namespace tornado
{

TcpServer::TcpServer():
    socket_(-1),
    io_loop_( IOLoop::instance() )
{
}

TcpServer::~TcpServer()
{
    stop();
}


void TcpServer::stop()
{
    if(socket_ >= 0)
    {
        io_loop_->removeHandler(socket_);
        ::close(socket_);
        socket_ = -1;        
    }
}

void TcpServer::start()
{
    io_loop_->start();
}

int TcpServer::listen(const std::string& host, uint16_t port)
{
    assert(socket_ < 0);
    socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ < 0)
    {
        LOG_ERROR("create socket fail errno_str = %s", STR_ERRNO);
        return -1;
    }
    assert( 0 == SocketUtil::setTcpnodealy(socket_) );
    assert( 0 == SocketUtil::setReuseaddr(socket_) );
    assert( 0 == SocketUtil::setNonblock(socket_) );

    struct sockaddr_in addr;  
    memset(&addr, 0x0, sizeof(addr));
   
    addr.sin_family = AF_INET;  
    addr.sin_port = htons(port);
    if(host.empty())
    {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);  
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr(host.c_str());
    }   
    
    if( ::bind(socket_, (struct sockaddr *)&addr, sizeof(addr) ) < 0 )
    {  
        LOG_ERROR("socket bind faild errno_str = %s", STR_ERRNO);
        return -1;
    }
    
    if(::listen(socket_, 128)<0)
    {  
        LOG_ERROR("socket listen faild errno_str = %s", STR_ERRNO);
        return -1;
    }
   
    io_loop_->addHandler(socket_, 
        boost::bind(&TcpServer::handleConnection, this, _1, _2),
        IOLoop::READ);

    LOG_INFO("socket(%d) listen and bind succ %s:%u", socket_, host.c_str(), port);

    return 0;
}


void TcpServer::handleConnection(int32_t socket, uint32_t events)
{
    //LOG_INFO("socket(%d)|socket=%d, events=%0x0x", socket_, socket, events);
    assert(socket_ == socket);
    while(true)
    {
        struct sockaddr_in  addr;
        memset((char*)&addr, 0x0, sizeof(addr));
        socklen_t addrlen = sizeof(addr);
        int fd  = accept4(socket_, (struct sockaddr*)&addr, &addrlen, SOCK_NONBLOCK);
        if ( fd < 0 )
        { 
            if(ERRNO_WOULDBLOCK)
            {
                LOG_INFO("ERRNO_WOULDBLOCK do nothing" );
                return ;
            }
            if(ERRNO_ECONNABORTED)
            {
                LOG_INFO("ERRNO_WOULDBLOCK continue" );
                continue;
            }
            LOG_ERROR("socket accept4 faild errno_str = %s", STR_ERRNO);
            return ;
        }
        assert(fd >= IOLoop::MIN_FD );

        LOG_INFO("socket(%d) accept|fd=%d|addr=%s:%d", socket_, fd, inet_ntoa(addr.sin_addr), addr.sin_port );
        
        IOStreamPtr  stream(new IOStream(fd));
        handleStream(stream);
        break;
     }
}

       

}//namespace
