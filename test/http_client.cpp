/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#include "Logging.h"
#include "TcpClient.h"
#include "HttpServer.h"

namespace tornado
{

class MyTCP : public TcpClient
{
    virtual void on_connect_succ(int32_t fd, int err)
    {
        TcpClient::on_connect_succ(fd, err);
        //set close callback
        char szbuff[400];
        memset(szbuff, 0x1, sizeof(szbuff));
        stream_->write_bytes(szbuff, 400, boost::bind(&MyTCP::write_done, this, _1));
    }

    void write_done(int32_t fd)
    {
        LOG_INFO_STR("connect timeout");
        stream_->read_bytes(16, boost::bind(&MyTCP::read_done, this, _1, _2));
    }
    
    void read_done(int32_t fd, const std::string& data)
    {
        static int i =0;
        i++;
        LOG_INFO("recv:%s", data.c_str());
        if(i>20)
        {
            io_loop_->addCallback(boost::bind(&MyTCP::quit_loop, this));
            return ;
        }
        //set close callback
        char szbuff[400];
        memset(szbuff, 0x1, sizeof(szbuff));
        stream_->write_bytes(szbuff, 400, boost::bind(&MyTCP::write_done, this, _1));
    }

    void quit_loop()
    {
    tornado::IOLoop::instance()->stop();
    }
    
};

}

void test(tornado::TcpClient* tcp)
{
     tcp->connect("10.12.16.139", 8888);
}

char buff[]="GET http://game.ld2.qq.com/cgi-bin/happy_fight_two?cmd=1&amp;gameflag=0&amp;petflag=0&amp;platform=kungfu&amp;localtime=1412818024496 HTTP/1.1\r\n"
    "Host: game.ld2.qq.com\r\n"
    "Connection: keep-alive\r\n"
    "User-Agent: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.153 Safari/537.36\r\n"
    "Accept: */*\r\n"
    "Referer: http://game.ld2.qq.com/\r\n"
    "Accept-Encoding: gzip,deflate,sdch\r\n"
    "Accept-Language: zh-CN,zh;q=0.8,en;q=0.6\r\n"
    "Cookie: piao_city=221; RK=Fd1e4D1kP1; pgv_pvi=315188224; ptcz=4886f4eb0249d1b13516c9cad798f5dfd45e5fcc5c0922440ef196ff44eec611; pt2gguin=o0979762787; o_cookie=979762787; pgv_pvid=1373164052\r\n"
    "\r\n"
    "\r\n";

#include<string>
#include <boost/algorithm/string.hpp>

int main()
{
   

/*
    tornado::MyTCP  objTCP;
    tornado::IOLoop::instance()->addCallback( boost::bind(&test, &objTCP) );
    tornado::IOLoop::instance()->start(); */

    std::string data(buff);
    size_t eol = data.find("\r\n");
       assert(eol != std::string::npos);
       //copy
       std::string start_line(data, 0, eol);
       std::vector<std::string> vItems;
       boost::split(vItems, start_line,  boost::is_any_of(" "));
       if(vItems.size() != 3)
       {
           //"Malformed HTTP request line"
            return -1;
       }
       //method, uri, version = 0 , 1, 2
       const std::string& method  = vItems[0];
       const std::string& uri     = vItems[1];
       const std::string& version = vItems[2];
       printf("%s|%s|%s\n", method.c_str(), uri.c_str(), version.c_str() );
       //StartsWith
       if(!version.find("HTTP/") == 0 )
       {
           printf("version find failed\n");
           return -1;
       }
       eol += 2; //skip \r\n
       tornado::HTTPHeaders  headers;
       int ret = headers.parse(data, eol);
       if(ret )
       {
           printf("headers.parse failed ret = %d\n", ret);
          return -2;
       }
       headers.print();


    return 0;
}