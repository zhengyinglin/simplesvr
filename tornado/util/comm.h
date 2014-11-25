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

namespace util
{

namespace comm
{

//16进制 123456789abcdef / ABCDEF
static const int8_t hex_char_to_int[256] =
  {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  , 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1
  ,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1
  ,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
  };

static const char  upper_hex[] = "0123456789ABCDEF";

std::string hex(const char* input, const size_t input_size)
{
    std::string output;
    output.reserve(input_size * 2);
    for(size_t i=0; i<input_size; i++)
    {
        char val = input[i];
        output.push_back( upper_hex[ (val >> 4) & 0x0F ] );
        output.push_back( upper_hex[ val & 0x0F ] );
    }
    return output;
}

std::string hex(const std::string& input)
{
    return  hex( input.c_str(), input.size() );
}

size_t hex(const char* input, const size_t input_size, char* buff, size_t buff_size)
{
    if( buff_size * 2 < input_size )
    {
        throw "buff_size * 2 < input_size";
    }
    size_t index = 0;
    for(size_t i=0; i<input_size; i++)
    {
        char val = input[i];
        buff[ index++ ] =  upper_hex[ (val >> 4) & 0x0F ];
        buff[ index++ ] =  upper_hex[ val & 0x0F ];
    }
    return index;
}


std::string unhex(const char* input, const size_t input_size)
{
    if( input_size & 0x1 ) // == input_size % 2 == 1
    {
        throw "input_size % 2 == 1";
    }
    std::string output;
    output.reserve(input_size/2);
    for(size_t i=0; i<input_size; i+=2)
    {
        int8_t val1 = input[i];
        int8_t val2 = input[i+1];
        char val =  (hex_char_to_int[val1] << 4 ) + hex_char_to_int[val2] ;
        output.push_back( val );
    }
    return output;
}

std::string unhex(const std::string& input)
{
    return  unhex( input.c_str(), input.size() );
}

size_t unhex(const char* input, const size_t input_size, char* buff, size_t buff_size)
{
    if( input_size & 0x1 ) // == input_size % 2 == 1
    {
        throw "input_size % 2 == 1";
    }
    if( buff_size < input_size/2 )
    {
        throw "buff_size < input_size/2";
    }
    size_t index = 0;
    for(size_t i=0; i<input_size; i+=2)
    {
        int8_t val1 = input[i];
        int8_t val2 = input[i+1];
        char val = (hex_char_to_int[val1] << 4 ) + hex_char_to_int[val2];
        buff[ index++ ] =  val;
    }
    return index;
}



time_t parseStrTime(const char* pszTime, const char* timeFmt="%Y-%m-%d %H:%M:%S")
{
    if(!pszTime) return 0;
    struct tm stTime;
    if (strptime(pszTime, timeFmt, &stTime) == NULL)
    {
        return 0;
    }
    return mktime(&stTime);
}



}//namespace comm

}//namespace util

#endif //_UTIL_COMM_COMM_H_
