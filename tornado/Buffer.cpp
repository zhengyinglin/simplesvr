#include "Buffer.h"
#include "Logging.h"
#include <assert.h>
#include <algorithm>

  
namespace tornado
{

Buffer::Buffer(size_t buff_len)
{    
    buff_len_ = buff_len;
    assert(buff_len_ > 0 && buff_len_ < 1024* 1024 * 10);
    read_index_ = 0 ;
    write_index_ = 0;
    buff_  = new char [buff_len_];
}

Buffer::~Buffer()
{
    delete []buff_;
}

void Buffer::clear()
{
    read_index_ = 0;
    write_index_ = 0;
}


bool Buffer::append(const char* data, size_t len)
{
    if( !ensureWritableBytes(len) )
        return false;
    std::copy(data, data + len,  end() );
    write_index_ += len;
    return true;
}

bool Buffer::ensureWritableBytes(size_t len)
{
    if (writableBytes() >= len)
    {
        return true;
    }
    if( writableBytes() + prependableBytes() < len )
    {
        return false;
    }
    makeSpace();
    assert( writableBytes() >= len );
    return true;
}
    
bool Buffer::extendReadBytes(size_t  add_bytes)
{
   if (writableBytes() < add_bytes)
   {
        return false;
   }
   write_index_ += add_bytes;
   return true;
}

void Buffer::makeSpace()
{
    LOG_INFO("--makeSpace read_index_ = %d write_index_=%d", (int)read_index_, (int)write_index_)
    assert(write_index_ >= read_index_ );

    size_t readable = readableBytes();
    std::copy(begin(),  end(),  buff_);
    read_index_ = 0;
    write_index_ = readable;
    LOG_INFO("++makeSpace read_index_ = %d write_index_=%d", (int)read_index_, (int)write_index_)
}



bool Buffer::readBytesToString(size_t len, std::string& result)
{
    if( len > readableBytes() )
        return false; 
    result.append(begin(), len);
    read_index_ += len;
    return true;
}

bool Buffer::readBytes(size_t len)
{
    if( len > readableBytes() )
        return false; 
    read_index_ += len;
    return true;
}

//返回其下标， <0 没有找到
int Buffer::find(const std::string& delimiter)
{
    const char* pos = std::search(begin(), end(), delimiter.begin(), delimiter.end());
    if(pos == end())
        return -1;
        
    return pos - begin();
}


}//namespace tornado