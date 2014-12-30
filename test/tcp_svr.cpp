/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/
#include "tornado/TcpServer.h"
#include "tornado/Logging.h"
#include "tornado/Util.h"
#include <signal.h>

namespace tornado
{


class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(IOStreamPtr stream):
        stream_(stream)
    {
        TORNADO_LOG_DEBUG_STR("--------->TcpConnection create stream");
    }

    virtual ~TcpConnection()
    {
        TORNADO_LOG_DEBUG_STR("<-------~TcpConnection release");
    }

    void startRun()
    { 
        stream_->setCloseCallback( std::bind(&TcpConnection::onStreamColse, shared_from_this()));
        startReading();
    }

    int startReading()
    {
        int iRet = stream_->readBytes(400, 
            std::bind(&TcpConnection::readHeader, shared_from_this(), std::placeholders::_1));
        if(iRet)
        {
            TORNADO_LOG_ERROR("readBytes faile ret = %d", iRet);
        }
        return iRet;
    }

    void readHeader(const std::string& data)
    {
        int fd = stream_->getFd();
        TORNADO_LOG_DEBUG("fd=%d|data=%s", fd, data.c_str());

        //std::string  buff(400, 'a');
        //int iRet = stream_->writeBytes(buff.c_str(), buff.size(), 
        int iRet = stream_->writeBytes(data.c_str(), data.size(), 
            std::bind(&TcpConnection::OnWriterDone, shared_from_this()) );
        if( iRet )
        {
            TORNADO_LOG_ERROR("fd=%d|writeBytes faile ret = %d", fd, iRet);
        }
    }
      
    void OnWriterDone()
    {
        TORNADO_LOG_DEBUG_STR("startReading again");
        startReading();
    }

    void onStreamColse()
    {
        TORNADO_LOG_DEBUG_STR("onStreamColse");
    }

private:
    IOStreamPtr  stream_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

class MySvr : public TcpServer
{
protected:
    virtual void handleStream(IOStreamPtr stream)
    {
        TORNADO_LOG_INFO_STR("handle_stream");
        TcpConnectionPtr ptr(new TcpConnection(stream));
        ptr->startRun();
    }
public:
    void signal_handler(int32_t fd, uint32_t events)
    {
        //must be remove 
        tornado::IOLoop::instance()->removeHandler(fd);
        TORNADO_LOG_WARN("Caught signal : will stop server signal fd=%d", fd);
        stop();
    }
};

}


#include "gflags/gflags.h"
DEFINE_int32(port, 8888, "--port=8888  listen port");


int main(int argc, char* argv[])
{
    tornado::init_log(argc, argv); //have ParseCommandLineFlags
    tornado::MySvr svr;

    int32_t sfd = tornado::IOLoop::instance()->addSignalHandler(SIGTERM, 
        std::bind(&tornado::MySvr::signal_handler, &svr, std::placeholders::_1, std::placeholders::_2)
        );
    if(sfd < 0)
    {
        TORNADO_LOG_ERROR_STR("addSignalHandler failed");
        return -1;
    }
    TORNADO_LOG_INFO("addSignalHandler fd=%d", sfd);

    if( svr.listen("", FLAGS_port) )//10.12.16.139
        return -1;

    svr.start();
    return 0;
}