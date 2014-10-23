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


class TcpConnection : public boost::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(IOStreamPtr& stream):
        stream_(stream)
    {
        LOG_DEBUG_STR("--------->TcpConnection create stream");
    }

    virtual ~TcpConnection()
    {
        LOG_DEBUG_STR("<-------~TcpConnection release");
    }

    void startRun()
    { 
        stream_->setCloseCallback( boost::bind(&TcpConnection::onStreamColse, shared_from_this()));
        startReading();
    }

    int startReading()
    {
        int iRet = stream_->readBytes(400, 
            boost::bind(&TcpConnection::readHeader, shared_from_this(), _1));
        if(iRet)
        {
            LOG_ERROR("readBytes faile ret = %d", iRet);
        }
        return iRet;
    }

    void readHeader(const std::string& data)
    {
        int fd = stream_->getFd();
        LOG_DEBUG("fd=%d|data=%s", fd, data.c_str());

        //std::string  buff(400, 'a');
        //int iRet = stream_->writeBytes(buff.c_str(), buff.size(), 
        int iRet = stream_->writeBytes(data.c_str(), data.size(), 
            boost::bind(&TcpConnection::OnWriterDone, shared_from_this()) );
        if( iRet )
        {
            LOG_ERROR("fd=%d|writeBytes faile ret = %d", fd, iRet);
        }
    }
      
    void OnWriterDone()
    {
        LOG_DEBUG_STR("startReading again");
        startReading();
    }

    void onStreamColse()
    {
        LOG_DEBUG_STR("onStreamColse");
    }

private:
    IOStreamPtr  stream_;
};

typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;

class MySvr : public TcpServer
{
protected:
    virtual void handleStream(IOStreamPtr& stream)
    {
        LOG_INFO_STR("handle_stream");
        TcpConnectionPtr ptr(new TcpConnection(stream));
        ptr->startRun();
    }
public:
    void signal_handler(int32_t fd, uint32_t events)
    {
        //must be remove 
        tornado::IOLoop::instance()->removeHandler(fd);
        LOG_WARN("Caught signal : will stop server signal fd=%d", fd);
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
        boost::bind(&tornado::MySvr::signal_handler, &svr, _1, _2)
        );
    if(sfd < 0)
    {
        LOG_ERROR_STR("addSignalHandler failed");
        return -1;
    }
    LOG_INFO("addSignalHandler fd=%d", sfd);

    if( svr.listen("", FLAGS_port) )//10.12.16.139
        return -1;

    svr.start();
    return 0;
}