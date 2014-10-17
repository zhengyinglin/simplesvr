/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/

#ifndef _TORNADO_BUFFER_H_
#define _TORNADO_BUFFER_H_
#include <sys/types.h>
#include <string>

namespace tornado
{

/// A buffer class modeled org.jboss.netty.buffer.ChannelBuffer
///
//      +-------------------+------------------+------------------+
//      | discardable bytes |  readable bytes  |  writable bytes  |
//      |                   |     (CONTENT)    |                  |
//      +-------------------+------------------+------------------+
//      |                   |                  |                  |
//      0      <=      read_index_   <=   write_index_    <=    capacity
///
class Buffer
{
public:
    Buffer(size_t buff_len = 1024 * 100);
    ~Buffer();

    void clear();
   
    inline const char*  data() const
    {
        return buff_ + read_index_;
    }
    inline char*  data() 
    {
        return buff_ + read_index_;
    }

    //该函数   data()  会失效
    bool ensureWritableBytes(size_t len);  
    bool extendReadBytes(size_t  add_bytes);


    inline size_t writableBytes() const
    { 
        return buff_len_ - write_index_;
    }
    
    inline size_t prependableBytes() const
    { 
        return read_index_; 
    }
    
    inline size_t readableBytes() const
    { 
        return write_index_ - read_index_; 
    }


    bool append(const char* data, size_t len);

    bool readBytesToString(size_t len, std::string& result);

    bool readBytes(size_t len);

    //返回其下标， <0 没有找到
    int find(const std::string& delimiter);

private: 
    void makeSpace();
    
    inline char*  begin() 
    {
        return buff_ + read_index_;
    }
    inline char*  end()
    {
        return buff_ + write_index_;
    }

private:
    char*        buff_;
    size_t       buff_len_;
    size_t       read_index_;
    size_t       write_index_;
};


  



}//namespace tornado

#endif  // _TORNADO_BUFFER_H_
