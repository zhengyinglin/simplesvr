/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#ifndef _TORNADO_TCPSERVER_H_
#define _TORNADO_TCPSERVER_H_

#include "IOStream.h"
#include <vector>

namespace tornado
{
class IOLoop;

class TcpServer
{
public:
    TcpServer();
    virtual ~TcpServer();

    int  listen(const std::string& host, uint16_t port);
    void stop();
    void start();

protected:
    void handleConnection(int32_t socket, uint32_t events);

    //虚函数--子类重载
    virtual void handleStream(IOStreamPtr& stream) = 0;

protected:
    int32_t  socket_;
    IOLoop*  io_loop_;
};


}//namespace tornado
#endif  // _TORNADO_TCPSERVER_H_
