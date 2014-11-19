#include "Buffer.h"
#include <assert.h>
#include <algorithm>

  
namespace tornado
{

Buffer::Buffer(size_t buff_len):
    buff_(NULL),
    buff_len_(buff_len),
    read_index_(0),
    write_index_(0)
{    
    assert(buff_len_ > 0 && buff_len_ < 1024* 1024 * 10);
    buff_  = new char [buff_len_];
    //buff_  = BufferMemoryManager::instance()->get(buff_len_);
}

Buffer::~Buffer()
{
   delete []buff_;
   //BufferMemoryManager::instance()->recover(buff_, buff_len_);
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
    assert(write_index_ >= read_index_ );

    size_t readable = readableBytes();
    std::copy(begin(),  end(),  buff_);
    read_index_ = 0;
    write_index_ = readable;
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



 /* 
//=================Buffer 对象里面的buff_ 内存管理类

BufferMemoryManager::~BufferMemoryManager()
{
    for(auto iter = mbuffs_.begin(); iter != mbuffs_.end(); ++iter)
    {
        for( auto i = iter->second.begin(); i != iter->second.end(); ++i )
        {
            delete (*i);
        }
    }
}

char* BufferMemoryManager::get(size_t buff_size)
{
    auto it = mbuffs_.find(buff_size);
    if( it != mbuffs_.end() )
    {
        auto& queue = it->second;
        if( !queue.empty() )
        {
            char* buff = queue.back();
            queue.pop_back();
            return buff;
        }
    }
    char* buff = new char [buff_size];
    return buff;
}

void BufferMemoryManager::recover(char* buff, size_t buff_size)
{
    auto it = mbuffs_.find(buff_size);
    if( it != mbuffs_.end() )
    {
        auto& queue = it->second;
        queue.push_back(buff);
        //add release here 
    }
    else
    {
        std::vector<char*>  queue;
        queue.reserve(128);
        queue.push_back(buff);
        mbuffs_[buff_size] = queue;
    }
}

BufferMemoryManager* BufferMemoryManager::instance()
{
    static BufferMemoryManager  _instance;
    return &_instance;
}
*/


}//namespace tornado