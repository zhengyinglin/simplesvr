/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#ifndef _TORNADO_HTTPSERVER_H_
#define _TORNADO_HTTPSERVER_H_

#include <map>
#include <string>
#include <memory>
#include "IOStream.h"
#include "TcpServer.h"

namespace tornado
{

class HTTPHeaders
{
public:
    
    int parse(const std::string& headers, size_t pos);
    int parseLine(const char* line, size_t len);
    int parseCookies();
    const std::string& getValue(const std::string& key) const ;
    int getValueInt(const std::string& key) const;

    void print() const ;
    const std::map<std::string, std::string>& getCookies()const {return cookies_;}

private:
    std::string              last_key_;
    std::string              last_value_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> cookies_;
};



struct HTTPRequest
{
    std::string method;
    std::string path;
    std::string query;
    std::string version;
    std::map<std::string, std::string> arguments;
    std::map<std::string, std::string> body_arguments;    
    HTTPHeaders headers;
    std::string body;

    void print() const
    {
        printf("method=%s\n", method.c_str());
        printf("path=%s\n", path.c_str());
        printf("query=%s\n", query.c_str());
        printf("version=%s\n", version.c_str());
        printf("body=%s\n", body.c_str());
        printf("method=%s\n", method.c_str());
        printf("===========>> arguments <<===========\n");
        for(auto iter = arguments.begin(); iter != arguments.end(); ++iter)
        {
            printf("[%s]:[%s]\n", iter->first.c_str(), iter->second.c_str());
        }
        printf("===========>> body_arguments <<===========\n");
        for(auto iter = body_arguments.begin(); iter != body_arguments.end(); ++iter)
        {
            printf("[%s]:[%s]\n", iter->first.c_str(), iter->second.c_str());
        }
        printf("===========>> HTTPHeaders <<===========\n");
        headers.print();
    }

};


class HTTPConnection;
typedef std::shared_ptr<HTTPConnection> HTTPConnectionPtr;
typedef std::function<void(HTTPConnectionPtr)> HttpRequestCallback;

//"""Handles a connection to an HTTP client, executing HTTP requests.
class HTTPConnection : public std::enable_shared_from_this<HTTPConnection>
{
public:
   HTTPConnection(IOStreamPtr& stream, HttpRequestCallback& request_callback);
   virtual ~HTTPConnection();

   void startRun();
private:
   void onStreamColse();
   void onHeaders(const std::string& data);
   void onRequestBody(const std::string& data);

public:
   void finish();
   int writeToBuff(const char* chunk, size_t len);
   int write(const std::string& chunk);
private:
   void onWriteComplete();


   int parseHttpRequest(const std::string& data);
   int parseBodyArguments(const std::string& body);
   int parseQsBytes(const std::string& qs, std::map<std::string, std::string>& arguments);
public:
   const HTTPRequest& getRequset() const { return request_; }
   void  setCloseCallback(IOLoop::Callback&& callback) { close_callback_ = callback; }
   int64_t  getRequestTimeMS() const ;

private:
    HTTPRequest  request_;
    IOStreamPtr  stream_;
    HttpRequestCallback  request_callback_;
    IOLoop::Callback     close_callback_;
    bool request_finished_;
    int64_t  start_time_;
    int64_t  finish_time_; 
};



class HTTPServer : public TcpServer
{
public:
    HTTPServer(HttpRequestCallback&& callback);
    virtual ~HTTPServer()
    {}

protected:
    //虚函数--子类重载
    virtual void handleStream(IOStreamPtr& stream);
private:
    HttpRequestCallback  callback_;
};



}//namespace tornado
#endif  // _TORNADO_HTTPSERVER_H_

