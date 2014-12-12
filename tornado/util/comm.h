/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-11-21
*/

#ifndef _UTIL_COMM_COMM_H_
#define _UTIL_COMM_COMM_H_

#include <stdint.h>
#include <string>
#include <chrono>
#include <iostream>

namespace util
{

namespace comm
{


std::string hex(const char* input, const size_t input_size);
std::string hex(const std::string& input);
size_t hex(const char* input, const size_t input_size, char* buff, size_t buff_size);


std::string unhex(const char* input, const size_t input_size);
std::string unhex(const std::string& input);
size_t unhex(const char* input, const size_t input_size, char* buff, size_t buff_size);


time_t parseStrTime(const char* pszTime, const char* timeFmt="%Y-%m-%d %H:%M:%S");


void encode_base64(std::string& dst, const std::string& src);
void encode_base64url(std::string& dst, const std::string& src);

int decode_base64(std::string& dst, const std::string& src);
int decode_base64url(std::string& dst, const std::string& src);



class TimeSpan
{
public:
    TimeSpan():
        tstart_( std::chrono::steady_clock::now() )
    {
        std::cout << "TimeSpan start ..." << std::endl;
    }
    ~TimeSpan()
    {
       auto tend = std::chrono::steady_clock::now();
       auto time_span = std::chrono::duration_cast< std::chrono::duration<double> >(tend - tstart_);
       std::cout << "TimeSpan end  it take seconds = " << time_span.count()  <<std::endl;
    }
private:
    std::chrono::steady_clock::time_point  tstart_;
};

}//namespace comm

}//namespace util

#endif //_UTIL_COMM_COMM_H_
