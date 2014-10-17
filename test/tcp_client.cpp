/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#include "tornado/Logging.h"
#include "tornado/IOStream.h"
#include "boost/enable_shared_from_this.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/bind.hpp"


namespace tornado
{

class TcpClient : public boost::enable_shared_from_this<TcpClient>
{
public:
    TcpClient():
        io_loop_(IOLoop::instance()),
        request_timer_(0),
        loopnum_(1),
        curnum_(0)
    {
        LOG_DEBUG("------>TcpClient create");
    }

    ~TcpClient()
    {
        LOG_DEBUG("<------~TcpClient exit");
    }

    bool connect(const char* ip, short port, int timeoutMS = 1000)
    {
        int socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (socket < 0)
        {
            LOG_ERROR("create socket fail errno_str = %s", STR_ERRNO);
            return false;
        }
        stream_.reset( new IOStream(socket) );
        //set close callback
        stream_->setCloseCallback(boost::bind(&TcpClient::onStreamClose, shared_from_this()) );

        //1秒超时
        request_timer_ = io_loop_->addTimeout(timeoutMS, boost::bind(&TcpClient::onConnectTimeout, shared_from_this(), _1, _2));

        return stream_->connect(ip, port, boost::bind(&TcpClient::connectDone, shared_from_this(), _1) );
    }

    virtual void connectDone(int err)
    {
        LOG_INFO("");  
        io_loop_->removeTimeout(request_timer_);
        if(err)
        {
            LOG_ERROR("connect socket fail");
            //stream_->close();
            //will close in stream error  auto  //stream_->clear();
            return ;
        }
        sendRequest();
    }

    virtual void onStreamClose()
    {
        LOG_INFO("");
        /*if(callback_)
        {
            callback_.clear();
        }*/
    }

    void sendRequest()
    {
        curnum_ ++;
        if(curnum_ > loopnum_)
        {
            stream_->close();
            return ;
        }
        char szbuff[400];
        memset(szbuff, 0xFF, sizeof(szbuff));
        stream_->writeBytes(szbuff, 400, boost::bind(&TcpClient::writeDone, shared_from_this()));
        request_timer_ = io_loop_->addTimeout(3000, boost::bind(&TcpClient::onRequestTimeout, shared_from_this(), _1, _2));
    }

    void writeDone()
    {
        LOG_INFO("");
        io_loop_->removeTimeout(request_timer_);
        stream_->readBytes(400, boost::bind(&TcpClient::readDone, shared_from_this(), _1));
        request_timer_ = io_loop_->addTimeout(3000, boost::bind(&TcpClient::onRequestTimeout, shared_from_this(), _1, _2));
    }
    
    void readDone(const std::string& data)
    {
        io_loop_->removeTimeout(request_timer_);
        LOG_INFO("");
        sendRequest();
    }
  
    void onConnectTimeout(IOLoop::TimerID tid, int64_t expiration)
    {
        LOG_WARN("onConnectTimeout timeout  close stream_");
        assert(tid == request_timer_);
        stream_->close();
    }
    void onRequestTimeout(IOLoop::TimerID tid, int64_t expiration)
    {
        LOG_WARN("onRequestTimeout timeout  close stream_");
        assert(tid == request_timer_);
        stream_->close();
    }
    
    void run(const std::string& ip, short port, int loopnum)
    {
        loopnum_ = loopnum;
        connect(ip.c_str(), port);
    }

private:
    IOLoop*          io_loop_;
    IOLoop::TimerID  request_timer_;
    IOStreamPtr      stream_;
    int              loopnum_; // 测试循环次数
    int              curnum_; // 成功次数
    /*IOStream::ReadCallback  callback_;*/
};

typedef boost::shared_ptr<TcpClient> TcpClientPtr;


}

int main(int argc, char** args)
{
    if( argc < 4 )//loopnum
    {
        printf("argc < 4\nusing ./tcp_client ip port loopnum  <log_level>\n");
        return -1;
    }
    std::string ip = args[1];
    short port = atoi(args[2]);
    int loopnum = atoi(args[3]);

    tornado::set_log_level(argc-3, args+3);

    tornado::TcpClientPtr tcp(new tornado::TcpClient());

    tornado::IOLoop::instance()->addCallback( 
        boost::bind(&tornado::TcpClient::run, tcp, 
                ip, port, loopnum) );
    tornado::IOLoop::instance()->start(); 
    return 0;
}