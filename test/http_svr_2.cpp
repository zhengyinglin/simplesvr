/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#include "tornado/Web.h"
#include "tornado/IOLoop.h"
#include "tornado/Logging.h"
#include <signal.h>
#include "tornado/Util.h"



#include "gflags/gflags.h"
DEFINE_int32(port, 8888, "--port=8888  listen port");

DEFINE_string(tcp_ip,  "", "--tcp_ip=172.0.0.1  get data svr ip");
DEFINE_int32(tcp_port, 8888, "--tcp_port=8888  get data svr port");


namespace tornado
{


class MainHandler : public RequestHandler
{
public:
    virtual void get()
    {
        LOG_INFO_STR("MainHandler");
        write("hello world");
        finish();
    }
    virtual RequestHandler* clone()
    {
        return new  MainHandler();
    }
};


class MyTCP : public boost::enable_shared_from_this<MyTCP>
{
public:
    MyTCP(IOStream::ReadCallback  cb):
        io_loop_(IOLoop::instance()),
        request_timer_(0),
        callback_ ( cb )
    {
        LOG_DEBUG_STR("------>MyTCP create");
    }

    ~MyTCP()
    {
        LOG_DEBUG_STR("------>~MyTCP exit");
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
        stream_->setCloseCallback(boost::bind(&MyTCP::onStreamClose, shared_from_this()) );

        //1秒超时
        request_timer_ = io_loop_->addTimeout(timeoutMS, boost::bind(&MyTCP::onConnectTimeout, shared_from_this(), _1, _2));

        return stream_->connect(ip, port, boost::bind(&MyTCP::connectDone, shared_from_this(), _1) );
    }

    virtual void connectDone(int err)
    {
        LOG_DEBUG_STR("");  
        io_loop_->removeTimeout(request_timer_);
        if(err)
        {
            LOG_ERROR_STR("connect socket fail");
            stream_->close();
            //will close in stream error  auto  //stream_->clear();
            return ;
        }
        char szbuff[400];
        memset(szbuff, 0xFF, sizeof(szbuff));
        stream_->writeBytes(szbuff, 400, boost::bind(&MyTCP::writeDone, this));

        request_timer_ = io_loop_->addTimeout(3000, boost::bind(&MyTCP::onRequestTimeout, shared_from_this(), _1, _2));
    }

    virtual void onStreamClose()
    {
        LOG_DEBUG_STR("");  
        //clear();

        if(callback_)
        {
            callback_.clear();
        }
        /*if(stream_)
        {
            stream_.reset();
        }*/
    }

    void writeDone()
    {
        LOG_DEBUG_STR("");  
        io_loop_->removeTimeout(request_timer_);
        stream_->readBytes(400, boost::bind(&MyTCP::readDone, this, _1));
        request_timer_ = io_loop_->addTimeout(3000, boost::bind(&MyTCP::onRequestTimeout, shared_from_this(), _1, _2));
    }
    
    void readDone(const std::string& data)
    {
        LOG_DEBUG_STR("");  
        io_loop_->removeTimeout(request_timer_);
        if(callback_)
        {
            callback_(data);
            stream_->close();
        }
    }

    void run()
    {
        connect(FLAGS_tcp_ip.c_str(), FLAGS_tcp_port);
    }

  
    void onConnectTimeout(IOLoop::TimerID tid, int64_t expiration)
    {
        LOG_WARN_STR("onConnectTimeout timeout  close stream_");
        assert(tid == request_timer_);
        stream_->close();
    }
    void onRequestTimeout(IOLoop::TimerID tid, int64_t expiration)
    {
         LOG_WARN_STR("onRequestTimeout timeout  close stream_");
        assert(tid == request_timer_);
        stream_->close();
    }

private:
    IOLoop*          io_loop_;
    IOLoop::TimerID         request_timer_;
    IOStream::ReadCallback  callback_;
    IOStreamPtr      stream_;
};
typedef boost::shared_ptr<MyTCP> MyTCPPtr;



class ExampleHandler : public RequestHandler
{
public:
    boost::shared_ptr<ExampleHandler> GetSharedThis() 
    { 
       return boost::dynamic_pointer_cast<ExampleHandler>( shared_from_this() ); 
    } 

    virtual void get()
    {
        LOG_INFO_STR("");
       // MyTCPPtr  tcp_;
        tcp_.reset( new MyTCP( boost::bind(&ExampleHandler::getDone, GetSharedThis(), _1)  ) );
        tcp_->run();
    }

    void getDone(const std::string& data)
    {
        LOG_INFO_STR("");
        write("hello world recv");
        write(data);
        finish();
    }

    virtual RequestHandler* clone()
    {
        return new  ExampleHandler();
    }
private:
    MyTCPPtr  tcp_;
};

}



int main(int argc, char** argv)
{
    tornado::init_log(argc, argv); //have ParseCommandLineFlags

    tornado::Application::HandlerMap  process;
    process["/"] = new tornado::MainHandler;
    process["/example"] = new tornado::ExampleHandler;

    tornado::Application app( process );

    int32_t sfd = tornado::IOLoop::instance()->addSignalHandler(SIGTERM, 
        boost::bind(&tornado::Application::signal_handler_stop, &app, _1, _2)
        );
    if(sfd < 0)
    {
        LOG_ERROR_STR("addSignalHandler failed");
        return -1;
    }
    LOG_INFO("addSignalHandler fd=%d", sfd);

    if( app.listen("", FLAGS_port) )//10.12.16.139
        return -1;
    tornado::IOLoop::instance()->start();
    return 0;
}