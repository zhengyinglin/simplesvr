#include "HttpServer.h"
#include "boost/algorithm/string.hpp"
#include "Logging.h"
#include <algorithm> 
#include "Util.h"


namespace tornado
{

//===================  HTTPHeaders =========================
static std::string CRLF("\r\n");
int HTTPHeaders::parse(const std::string& headers, size_t pos)
{
    //http 一定有以 \r\n 结束行的
    while(pos < headers.size())
    {
        size_t  index = headers.find(CRLF, pos);
        if(index == std::string::npos)
        {
            //log warning ...
            TORNADO_LOG_WARN_STR("parse warnning ....");
            break;
        }
        if(index > pos)
        {
            int ret = parseLine( headers.c_str() + pos,  index - pos );
            if(ret)
            {
                return ret;
            }
        }
        pos = index + CRLF.size();
    }
    //parse cookies
    if( parseCookies() )
    {
        TORNADO_LOG_WARN_STR("parseCookies fail");
        return 100;
    }
   
    return 0;
}

//maybe modify
int HTTPHeaders::parseLine(const char* line, size_t len)
{
    //len > 0
    const char* end = line + len;

    //如果有2个一样的后面那个覆盖前面那个
    if(line[0] == ' ')//isspace # continuation of a multi-line header
    {
        TORNADO_LOG_WARN_STR("continuation of a multi-line header");
        if(last_key_.empty())
        {
            return -1;
        }
        std::string ext_value(line, len);
        boost::algorithm::trim_left(ext_value);
        last_value_.append( " " );
        last_value_.append( ext_value );
        headers_[ last_key_ ] = last_value_;
    }
    else
    {
        const char* sem = std::find(line, end, ':');
        if(sem == end)
        {
            return -2;
        }
        last_key_.assign(line, sem-line);
        last_value_.assign(sem + 1, end - sem - 1);
        boost::algorithm::trim(last_value_) ;
        headers_[ last_key_ ] = last_value_;
    }
    return 0;
}

int HTTPHeaders::parseCookies()
{
    const std::string& cookies_buff = getValue("Cookie");
    if(cookies_buff.empty())
    {
        return 0;
    }
    
    std::vector<std::string> vItems;
    boost::split(vItems, cookies_buff, boost::is_any_of(";"));
    std::vector<std::string> vArgs;
    for(auto iter=vItems.begin(); iter != vItems.end(); ++iter)
    {
        vArgs.clear();
        boost::algorithm::trim( *iter ) ;
        boost::split(vArgs, *iter, boost::is_any_of("="));
        if(vArgs.size() != 2)
        {
            TORNADO_LOG_WARN("%s num %zu!= 2", iter->c_str(), vArgs.size() );
            return -1;
        }
        cookies_[ vArgs[0] ] = vArgs[1]; 
    }
    return 0;
}

    


const std::string& HTTPHeaders::getValue(const std::string& key) const
{
    auto iter = headers_.find(key);
    if(iter == headers_.end())
    {
        static std::string empty_str("");
        return empty_str;
    }
    return iter->second;
}

int HTTPHeaders::getValueInt(const std::string& key) const
{
    auto iter = headers_.find(key);
    if(iter == headers_.end())
    {
        return 0;
    }
    return atoi(iter->second.c_str());
}

void HTTPHeaders::print() const 
{
    for(auto iter = headers_.begin(); iter != headers_.end(); ++iter)
    {
        printf("[%s]:[%s]\n", iter->first.c_str(), iter->second.c_str());
    }
    printf(".....cookies.....\n");
    
    for(auto iter = cookies_.begin(); iter != cookies_.end(); ++iter)
    {
        printf("cookies\t\t[%s]:[%s]\n", iter->first.c_str(), iter->second.c_str());
    }
}
 


//===================  HTTPConnection =========================
HTTPConnection::HTTPConnection(IOStreamPtr& stream, HttpRequestCallback& request_callback):
    stream_(stream), request_callback_(request_callback), request_finished_(false),
    start_time_( TimeUtil::curTimeMS() ), finish_time_(0)
{
    TORNADO_LOG_DEBUG_STR("-------->HTTPConnection create");
}

HTTPConnection::~HTTPConnection()
{
    TORNADO_LOG_DEBUG_STR("<--------~HTTPConnection release");
}

void HTTPConnection::onStreamColse()
{
    TORNADO_LOG_DEBUG_STR("stream close --> onConnectionClose");

    if(close_callback_)
    {
        close_callback_();
        close_callback_.clear();
    }
}

void HTTPConnection::startRun()
{
    stream_->setCloseCallback( boost::bind(&HTTPConnection::onStreamColse, shared_from_this()));

    int iRet = stream_->readUntil("\r\n\r\n", boost::bind(&HTTPConnection::onHeaders, shared_from_this(), _1) );
    if(iRet)
    {
        TORNADO_LOG_WARN("read_until fail ret=%d", iRet);
    }
}


void HTTPConnection::onHeaders(const std::string& data)
{
    TORNADO_LOG_DEBUG("data=%s", data.c_str());
    int ret = parseHttpRequest(data);
    if(ret)
    {
        TORNADO_LOG_WARN("close stream_ parseHttpRequest fail ret = %d", ret);
        stream_->close();
        return ;
    }
    int content_length = request_.headers.getValueInt("Content-Length");
    if(content_length > 0 )
    {
         if( content_length > 1024 * 100 )
         {
             TORNADO_LOG_WARN("close stream_ Content-Lengthtoo long size =%d", content_length);
             stream_->close();
             return ;
         }
         if( request_.headers.getValue("Expect") == "100-continue")
         {
             stream_->writeBytes("HTTP/1.1 100 (Continue)\r\n\r\n");
         }
         stream_->readBytes(content_length, boost::bind(&HTTPConnection::onRequestBody, shared_from_this(), _1));
         return;
    }
    TORNADO_LOG_DEBUG_STR("Content-Length 0 run callback");
    request_callback_(shared_from_this());
}

void HTTPConnection::onRequestBody(const std::string& data)
{
    request_.body = data;
    if(request_.method == "POST") //, "PATCH", "PUT"):
    {
        if( parseBodyArguments(data) )
        {
            TORNADO_LOG_WARN_STR("parse_body_arguments failed close stream");
            stream_->close();
            return ;
        }
    }
    TORNADO_LOG_DEBUG("run callback method=%s", request_.method.c_str());
    request_callback_(shared_from_this());
}

int HTTPConnection::parseHttpRequest(const std::string& data)
{
    size_t eol = data.find(CRLF);
    if(eol == std::string::npos)
    {
        TORNADO_LOG_WARN_STR("can not find CRLF");
        return -1;
    }
    //copy
    std::string start_line(data, 0, eol);

    TORNADO_LOG_DEBUG("start_line:%s", start_line.c_str());

    std::vector<std::string> vItems;
    boost::split(vItems, start_line, boost::is_any_of(" "));
    if(vItems.size() != 3)
    {
        TORNADO_LOG_WARN("Malformed HTTP request start line should 3 item but %zu", vItems.size() );
        return -2;
    }
    
    //method, uri, version = 0 , 1, 2
    request_.method = vItems[0];
    const std::string& uri  = vItems[1];
    request_.version = vItems[2];

    if(!boost::starts_with(request_.version, "HTTP/") )
    {
        TORNADO_LOG_WARN(" in HTTP Request-Line _BadRequestException:Malformed HTTP version %s", request_.version.c_str() );
        return -3;
    }

    {
        //parse uri
        std::vector<std::string> vItems2;
        boost::split(vItems2, uri, boost::is_any_of("?"));
        if(vItems2.size() > 2)
        {
            TORNADO_LOG_WARN("parse uri failed have num %zu > 2", vItems2.size()  );
            return -4;
        }
        if(vItems2.size() >= 1)
        {
            request_.path = vItems2[0];
        }
        if(vItems2.size() >= 2)
        {
            request_.query = vItems2[1];
            if( parseQsBytes(request_.query, request_.arguments) )
            {
                TORNADO_LOG_WARN_STR("parseQsBytes failed");
                return -5;
            }
        }
    }

    eol += CRLF.size(); //skip \r\n
    int ret =request_.headers.parse(data, eol);
    if(ret )
    {
        TORNADO_LOG_WARN("Malformed HTTP headers headers.parse failed ret =%d", ret);
        return -6;
    }

    return 0;
}

//Parses a form request body. Supports ``application/x-www-form-urlencoded``
int HTTPConnection::parseBodyArguments(const std::string& body)
{
    const std::string& content_type = request_.headers.getValue("Content-Encoding");
    
    if(boost::starts_with(content_type, "application/x-www-form-urlencoded") )
    {
        if( parseQsBytes(body, request_.body_arguments) )
        {
            TORNADO_LOG_WARN("parseQsBytes fail Invalid x-www-form-urlencoded body: %s", body.c_str());
            return -1;
        }
    }
    else 
    {
        TORNADO_LOG_WARN("Unsupported Content-Encoding:  %s", content_type.c_str());
    }
    return 0;
}

int HTTPConnection::parseQsBytes(const std::string& qs, std::map<std::string, std::string>& arguments)
{
    std::vector<std::string> vItems;
    boost::split(vItems, qs, boost::is_any_of("&"));
    std::vector<std::string> vArgs;
    for(auto iter=vItems.begin(); iter != vItems.end(); ++iter)
    {
        vArgs.clear();
        boost::split(vArgs, *iter, boost::is_any_of("="));
       
        if(vArgs.size() != 2)
        {
            TORNADO_LOG_WARN("=num %zu != 2", vArgs.size());
            return -1;
        }
        arguments[ vArgs[0] ] = vArgs[1]; 
    }
    return 0;
}


//"""Finishes the request."""
void HTTPConnection::finish()
{
    finish_time_ =  TimeUtil::curTimeMS();
    request_finished_ = true;
    if(!stream_->writing())
    {
       TORNADO_LOG_INFO_STR("OK close stream");
       stream_->close();
    }
}
int64_t  HTTPConnection::getRequestTimeMS() const 
{
    if(finish_time_)
    {
        return finish_time_ - start_time_;
    }
    return TimeUtil::curTimeMS() - start_time_;
}

int HTTPConnection::write(const std::string& chunk)
{
    return stream_->writeBytes(chunk.c_str(), chunk.size(),  boost::bind(&HTTPConnection::onWriteComplete, this) );
}

int HTTPConnection::writeToBuff(const char* chunk, size_t len)
{
    return stream_->justWriteBytesToBuff(chunk, len);
}



void HTTPConnection::onWriteComplete()
{
    if(request_finished_ && !stream_->writing() && !stream_->closed() )
    {
        TORNADO_LOG_INFO_STR("OK close stream");
        stream_->close();
    }
}
 




//===================  HTTPServer =========================
HTTPServer::HTTPServer(HttpRequestCallback  callback):
    TcpServer(),
    callback_(callback)
{
}


void HTTPServer::handleStream(IOStreamPtr& stream)
{
    const int32_t fd = stream->getFd();
    TORNADO_LOG_DEBUG("fd=%d", fd);
    //此处基于IOStreamPtr  与 HTTPConnectionPtr 的循环引用 来保证 2个对象处理过程中不被释放
    //断开循环引用基于IOStream::close（转移到IOLoop）, 释放对HTTPConnection的引用，
    //HTTPConnection 对象析构  、IOStream对象析构
    HTTPConnectionPtr ptr(new HTTPConnection(stream, callback_));
    ptr->startRun();
}


}//namespace
