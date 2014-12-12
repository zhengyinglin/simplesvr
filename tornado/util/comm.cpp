#include "comm.h"

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



time_t parseStrTime(const char* pszTime, const char* timeFmt/*="%Y-%m-%d %H:%M:%S"*/)
{
    if(!pszTime) return 0;
    struct tm stTime;
    if (strptime(pszTime, timeFmt, &stTime) == NULL)
    {
        return 0;
    }
    return mktime(&stTime);
}



//from nginx-1.6.0/src/core/ngx_string.c  
static void encode_base64_internal(std::string& dst, const char* src, const size_t src_len, const char *basis,  bool padding)
{
    size_t  len = src_len;
    const char* s = src;
    dst.clear();
    dst.reserve( len/3 * 4 + 4); //may be max len 

    while (len > 2) 
    {
        dst += basis[(s[0] >> 2) & 0x3f];
        dst += basis[((s[0] & 3) << 4) | (s[1] >> 4)];
        dst += basis[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
        dst += basis[s[2] & 0x3f];

        s += 3;
        len -= 3;
    }

    if (len) 
    {
        dst += basis[(s[0] >> 2) & 0x3f];

        if (len == 1) 
        {
            dst += basis[(s[0] & 3) << 4];
            if (padding) 
            {
                dst += '=';
            }
        } 
        else 
        {
            dst += basis[((s[0] & 3) << 4) | (s[1] >> 4)];
            dst += basis[(s[1] & 0x0f) << 2];
        }
        if (padding) 
        {
            dst += '=';
        }
    }
}

void encode_base64(std::string& dst, const std::string& src)
{
    static char basis64[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    encode_base64_internal(dst, src.data(), src.length(), basis64, true);
}


void encode_base64url(std::string& dst, const std::string& src)
{
    static char basis64[] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

    encode_base64_internal(dst, src.data(), src.length(), basis64, false);
}


static int decode_base64_internal(std::string& dst, const char* src, const size_t src_len, const int *basis)
{
    size_t  len = 0;
    for(len = 0; len < src_len; len++)
    {
        if (src[len] == '=')
        {
            break;
        }

        if (basis[(int)src[len]] == 77) 
        {
            return -1;
        }
    }

    if (len % 4 == 1) 
    {
        return -2;
    }

    const char *s = src;
    dst.clear();
    dst.reserve( len/4 * 3 + 2); //may be max len 

    while (len > 3)
    {
        dst += (char) (basis[(int)s[0]] << 2 | basis[(int)s[1]] >> 4);
        dst += (char) (basis[(int)s[1]] << 4 | basis[(int)s[2]] >> 2);
        dst += (char) (basis[(int)s[2]] << 6 | basis[(int)s[3]]);

        s += 4;
        len -= 4;
    }
    if (len > 1) {
        dst += (char) (basis[(int)s[0]] << 2 | basis[(int)s[1]] >> 4);
    }

    if (len > 2) {
        dst += (char) (basis[(int)s[1]] << 4 | basis[(int)s[2]] >> 2);
    }
    return 0;
}


int decode_base64(std::string& dst, const std::string& src)
{
    static int basis64[] = {
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
        77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
        77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };

    return decode_base64_internal(dst, src.data(), src.length(), basis64);
}

int decode_base64url(std::string& dst, const std::string& src)
{
    static int  basis64[] = {
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
        77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 63,
        77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };

    return decode_base64_internal(dst, src.data(), src.length(), basis64);
}



}//namespace comm

}//namespace util
