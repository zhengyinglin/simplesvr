#include "Web.h"
#include <algorithm>
#include "Logging.h"
#include "Util.h"

namespace tornado
{


HTTPCode::HTTPCode()
{
    for(int i=0; i<MAX_CODE; i++)
    {
        httpCodeMsg_[i] = NULL;
    }
    httpCodeMsg_[0] = "Unknown" ; 
    httpCodeMsg_[100] = "Continue" ;
    httpCodeMsg_[101] = "Switching Protocols" ;
    httpCodeMsg_[200] = "OK" ;
    httpCodeMsg_[201] = "Created" ;
    httpCodeMsg_[202] = "Accepted" ;
    httpCodeMsg_[203] = "Non-Authoritative Information" ;
    httpCodeMsg_[204] = "No Content" ;
    httpCodeMsg_[205] = "Reset Content" ;
    httpCodeMsg_[206] = "Partial Content" ;
    httpCodeMsg_[300] = "Multiple Choices" ;
    httpCodeMsg_[301] = "Moved Permanently" ;
    httpCodeMsg_[302] = "Found" ;
    httpCodeMsg_[303] = "See Other" ;
    httpCodeMsg_[304] = "Not Modified" ;
    httpCodeMsg_[305] = "Use Proxy" ;
    httpCodeMsg_[306] = "(Unused)" ;
    httpCodeMsg_[307] = "Temporary Redirect" ;
    httpCodeMsg_[400] = "Bad Request" ;
    httpCodeMsg_[401] = "Unauthorized" ;
    httpCodeMsg_[402] = "Payment Required" ;
    httpCodeMsg_[403] = "Forbidden" ;
    httpCodeMsg_[404] = "Not Found" ;
    httpCodeMsg_[405] = "Method Not Allowed" ;
    httpCodeMsg_[406] = "Not Acceptable" ;
    httpCodeMsg_[407] = "Proxy Authentication Required" ;
    httpCodeMsg_[408] = "Request Timeout" ;
    httpCodeMsg_[409] = "Conflict" ;
    httpCodeMsg_[410] = "Gone" ;
    httpCodeMsg_[411] = "Length Required" ;
    httpCodeMsg_[412] = "Precondition Failed" ;
    httpCodeMsg_[413] = "Request Entity Too Large" ;
    httpCodeMsg_[414] = "Request-URI Too Long" ;
    httpCodeMsg_[415] = "Unsupported Media Type" ;
    httpCodeMsg_[416] = "Requested Range Not Satisfiable" ;
    httpCodeMsg_[417] = "Expectation Failed" ;
    httpCodeMsg_[500] = "Internal Server Error" ;
    httpCodeMsg_[501] = "Not Implemented" ;
    httpCodeMsg_[502] = "Bad Gateway" ;
    httpCodeMsg_[503] = "Service Unavailable" ;
    httpCodeMsg_[504] = "Gateway Timeout" ;
    httpCodeMsg_[505] = "HTTP Version Not Supported" ;

}

const char* HTTPCode::toStr(int code) const
{
    if(code >= 0 && code <= MAX_CODE && httpCodeMsg_[ code ])
    {
        return httpCodeMsg_[ code ];
    }
    return httpCodeMsg_[ 0 ];//"Unknown" ; 
}




//===================== RequestHandler ===========================
RequestHandler::RequestHandler():
        finished_(false),
        status_code_(200)
{
    LOG_DEBUG("------> RequestHandler create")
}

RequestHandler::~RequestHandler()
{
    LOG_DEBUG("<------- RequestHandler release")
}

void RequestHandler::setHTTPConnection(const HTTPConnectionPtr&  conn)
{
    conn_ = conn;
    conn_->setCloseCallback( boost::bind(&RequestHandler::on_connect_close, shared_from_this() ) );
}

void RequestHandler::on_connect_close()
{
    LOG_DEBUG("on_connect_close")
}

void RequestHandler::write(const std::string& chunk)
{
    if (finished_)
    {
        LOG_ERROR("Cannot write() after finish()")
        return ;
    }        
    buff_.append(chunk);
}

void RequestHandler::write(const char* chunk, size_t len)
{
    if (finished_)
    {
        LOG_ERROR("Cannot write() after finish()")
        return ;
    }        
    buff_.append(chunk, len);
}



void RequestHandler::send_error(int status_code)
{ 
    status_code_ = status_code;
    char buf[256];
    int index = snprintf(buf, sizeof buf, "<html><title>%d: %s</title>",
        status_code_, HTTPCode::instance().toStr(status_code_));
    write(buf, index);
    finish();
}


void RequestHandler::finish()
{
    LOG_DEBUG("");
    //todo add 304 support
    if(finished_)
    {
        LOG_ERROR("finish() called twice")
        return ;
    }
    finished_ = true;
    flush();
    conn_->finish();
    _log();
    
}
    
void RequestHandler::flush()
{
    if(conn_)
    {
        //self.set_header("Content-Type", "application/json; charset=UTF-8")
        char buf[256];
        int size = snprintf(buf, sizeof buf, 
                "HTTP/1.1 %d %s\r\n"
                "Server: cppsvr\r\n"
                "Date: %s\r\n"
                "Content-Type: text/html; charset=utf-8\r\n"
                "Content-Length: %zd\r\n\r\n",
                status_code_, HTTPCode::instance().toStr(status_code_), 
                TimeUtil::getGMTTimestamp(), 
                buff_.size());
        
        conn_->writeToBuff(buf, size);
        conn_->write(buff_);
        buff_.clear();
    }
}

void RequestHandler::execute()
{
    if(finished_)
    {
        LOG_ERROR("finish() called twice")
        return ;
    }
    const std::string& method = conn_->getRequset().method;
    if(method == "GET")
    {
        get();
    }
    else if(method == "POST")
    {
        //self.application.settings.get("xsrf_cookies"):
        //# If XSRF cookies are turned on, reject form submissions without the proper cookie
        //self.check_xsrf_cookie()
        post();
    }
    else
    {
        send_error(405);
        return ;
    }
}


void RequestHandler::_log()
{
   const HTTPRequest& request = conn_->getRequset();

   LOG_INFO("%s %s %s|%d %s %.3fms", request.method.c_str(), request.path.c_str(), request.query.c_str(), 
       status_code_, HTTPCode::instance().toStr(status_code_), conn_->getRequestTimeMS()/1000.0 );
}


const std::string& RequestHandler::get_argument(const std::string& name)
{
    const HTTPRequest& request = conn_->getRequset();
    return _get_argument(name, request.arguments);
}

const std::string& RequestHandler::get_body_argument(const std::string& name)
{
    const HTTPRequest& request = conn_->getRequset();
    return _get_argument(name, request.body_arguments);
}
    
const std::string& RequestHandler::_get_argument(const std::string& name, const std::map<std::string, std::string>& argmap)
{
    auto it = argmap.find(name);
    if( it == argmap.end() )
    {
        static std::string  emtpyStr("");
        return emtpyStr;
    }
    return it->second;
}

const std::string& RequestHandler::get_cookie(const std::string& name)
{
    const HTTPRequest& request = conn_->getRequset();
    return _get_argument(name, request.headers.getCookies());
} 
  

//===========================Application====================

int Application::listen(const std::string& address, uint32_t port)
{
    assert( NULL == server_);
    server_ = new HTTPServer( boost::bind(&Application::handler, this, _1) ); 
    if( server_->listen(address, port))
        return -1;
    return 0;
}

void Application::stop()
{
    if(server_)
        server_->stop();
}
    
void Application::signal_handler_stop(int32_t fd, uint32_t events)
{  
    LOG_WARN('Caught signal : will stop http application signal fd=%d', fd);
    //must be remove 
    IOLoop::instance()->removeHandler(fd);
    stop();
}

void Application::handler(HTTPConnectionPtr conn)
{
    const HTTPRequest& request = conn->getRequset();

    typedef boost::shared_ptr<RequestHandler> RequestHandlerPtr;

    RequestHandlerPtr handler( find_handler(request.path) );
    handler->setHTTPConnection(conn);
    handler->execute();
}

RequestHandler* Application::find_handler(const std::string& path)
{
    auto handler = handlers_.find(path);
    if( handler == handlers_.end())
    {
        return new ErrorHandler();
    }
    return handler->second->clone();//new one 
}



}//namespace tornado
