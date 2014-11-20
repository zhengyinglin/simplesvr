/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#include "tornado/HttpServer.h"
#include "tornado/Logging.h"
#include "tornado/Util.h"
#include <signal.h>



void handler(tornado::HTTPConnectionPtr conn)
{
    const tornado::HTTPRequest& request = conn->getRequset();
    TORNADO_LOG_INFO("%s %s %s", request.method.c_str(), request.path.c_str(), request.query.c_str());

    conn->write("HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain; charset=utf-8\r\n"
        "Server: mysvr\r\n"
        "Date: Wed, 15 Oct 2014 08:21:07 GMT\r\n"
        "Connection: close\r\n"
        "Content-Length: 2\r\n\r\n"
        "OK");
    conn->finish();
}


//void shutdown(int32_t fd)
//{
//    LOG_INFO('Stopping server');
//        self.stop()
//        applog.info('Will shutdown max %d seconds ...', conf.MaxStopTime)
//        deadline = time.time() + conf.MaxStopTime
//        def stop_loop():
//            ioloop = IOLoop.instance()
//            now = time.time()
//            if now < deadline and not self._isfinished(ioloop):
//                ioloop.add_timeout(now + 0.5, stop_loop)       
//            else:
//                ioloop.stop()
//            self.app.dump_file() #will stop shold dump_file  
//            applog.info('Shutdown done')
//        self.stop_timer()
//        stop_loop()
//
// def _isfinished(self, ioloop):
//        if ioloop._callbacks :
//            return False
//        for t in ioloop._timeouts:
//            if t.callback:
//                return False
//        return True
//}

void signal_handler(int32_t fd, uint32_t events)
{
    tornado::IOLoop::instance()->removeHandler(fd);
    TORNADO_LOG_WARN("Caught signal : will stop server fd=%d", fd);
    tornado::IOLoop::instance()->stop();
}

void signal_handler_2(tornado::HTTPServer* svr, int32_t fd, uint32_t events)
{
    //must be remove 
    tornado::IOLoop::instance()->removeHandler(fd);
    TORNADO_LOG_WARN("Caught signal : will stop server signal fd=%d", fd);
    svr->stop();
}


#include "gflags/gflags.h"
DEFINE_int32(port, 8888, "--port=8888  listen port");

int main(int argc, char** argv)
{
    bool ret = tornado::init_log(argc, argv);//have ParseCommandLineFlags
    if(!ret)
    {
        perror("init_log faid");
    }
    tornado::HTTPServer svr( &handler );

    int32_t sfd = tornado::IOLoop::instance()->addSignalHandler(SIGTERM, 
        //&signal_handler
        std::bind(&signal_handler_2, &svr, std::placeholders::_1, std::placeholders::_2)
        );

    if(sfd < 0)
    {
        TORNADO_LOG_ERROR("addSignalHandler failed");
        return -1;
    }
    TORNADO_LOG_INFO("addSignalHandler fd=%d", sfd);

    if( svr.listen("", FLAGS_port) )//
        return -1;
    svr.start();
    return 0;
}