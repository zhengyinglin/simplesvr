/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-12-12
*/

#include "tornado/Logging.h"
#include "tornado/Resolver.h"
#include <functional>
#include <string>


void getips(int err, const tornado::Resolver::Hosts* hosts)
{
    for(auto& host : *hosts)
    {
        TORNADO_LOG_INFO("host|%s", host.c_str());
    }    
}

void gethosts(tornado::Resolver* ch, const std::string& hostname)
{
    ch->gethostbyname(getips, hostname.c_str());
}



#include "gflags/gflags.h"
DEFINE_string(hostname,  "localhost", "host name eg[qq.com]");

int main(int argc, char* argv[])
{
    tornado::init_log(argc, argv);//have ParseCommandLineFlags
    
    tornado::Resolver  r;
    TORNADO_LOG_INFO("init = %d", r.init()); 

      
    tornado::IOLoop::instance()->addCallback( 
        std::bind(&gethosts, &r, std::ref(FLAGS_hostname))
        );

  
    tornado::IOLoop::instance()->start(); 

    return 0;
}