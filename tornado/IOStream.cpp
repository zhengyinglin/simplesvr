#include "IOStream.h"
#include <string.h>
#include <assert.h>
#include <algorithm>
#include "Util.h"
#include "Logging.h"


namespace tornado
{

typedef std::shared_ptr<std::string> StringPtr;

void inner_connect_callback(IOStream::ConnectCallback& cb, int err)
{
    cb(err);
}

void inner_read_callback(IOStream::ReadCallback& cb, StringPtr data)
{
    cb(*data);
}

IOStream::IOStream(int32_t socket):
        socket_(socket),
        connecting_(false),
        closed_(false),
        state_(0),
        read_bytes_(0),
        read_until_close_(false)
{
    assert(socket_ >= IOLoop::MIN_FD );
    SocketUtil::setNonblock(socket_);
    SocketUtil::setTcpnodealy(socket_);
    io_loop_ = IOLoop::instance();
    TORNADO_LOG_DEBUG("--------->IOStream create  fd=%d", socket_);
}

IOStream::~IOStream()
{
    TORNADO_LOG_DEBUG("<---------IOStream release fd=%d", socket_);
    this->close();
}

void IOStream::close()
{
    TORNADO_LOG_DEBUG("socket_=%d|closed_=%d", socket_, closed_);
    if(closed_)
    {
        return ;
    }
    closed_ = true;

    if(read_until_close_)
    {
        read_until_close_ = false;
        size_t buffsize = read_buffer_.readableBytes();
        if(buffsize > 0)
        {
            StringPtr result(new std::string);
            if( read_buffer_.readBytesToString(buffsize, *result) )
            {
                if( read_callback_ )
                {
                    TORNADO_LOG_INFO_STR("recv data run read_callback_ put to ioloop callback");
                    io_loop_->addCallback( std::bind(&inner_read_callback, read_callback_, result) );
                    read_callback_ = nullptr;
                }
            }
        }
    }

    if(state_ != 0)
    {
        io_loop_->removeHandler(socket_);
        state_ = 0;
    }

    assert(socket_ >= IOLoop::MIN_FD );
    ::close(socket_);
    TORNADO_LOG_INFO("close socket fd =%d",  socket_);
    if(close_callback_)
    {
        TORNADO_LOG_INFO_STR("run close_callback put to ioloop callback");
        io_loop_->addCallback( std::move(close_callback_) );
        close_callback_ = nullptr; //release
    }
    
    if(connect_callback_)
        connect_callback_ = nullptr;
    if(read_callback_)
        read_callback_ = nullptr;
    if(write_callback_)
        write_callback_ = nullptr;
}


bool IOStream::connect(const char* ip, short port)
{
    assert(socket_ >= IOLoop::MIN_FD );
    connecting_ = true;
    struct sockaddr_in  addr;
    memset(&addr, 0x0, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip); //inet_addr转换为网络字节序

    int ret = ::connect(socket_,  (struct sockaddr*)&addr, sizeof(struct sockaddr) );
    if(ret != 0 )
    {
       if ( !ERRNO_INPROGRESS && !ERRNO_WOULDBLOCK )
       {
           TORNADO_LOG_ERROR("Connect error on fd:%d, strerron:%s", socket_, STR_ERRNO);
           this->close();
           return false;
       }
    }
    addIOState(IOLoop::WRITE);
    return true;
}

bool IOStream::connect(const char* ip, short port, ConnectCallback callback)
{
    if(!connect(ip, port))
        return false;

    connect_callback_ = callback;
    return true;
}


void IOStream::setCloseCallback(CloseCallback&& callback)
{
    close_callback_ = callback;
}


int IOStream::writeBytes(const char* data, int len)
{
    if(closed_)
        return -1;
    if(len <= 0)
        return -2;

    if(false == write_buffer_.append(data, len) )
    { 
        TORNADO_LOG_ERROR_STR("Reached maximum read buffer size");
        this->close();
        return -3;
    }

    if(!connecting_)
    {
        if( handleWrite())
        {
            TORNADO_LOG_ERROR_STR("handleWrite failed");
            this->close();
            return -4;
        }
        if(write_buffer_.readableBytes() > 0 )
        {
            addIOState(IOLoop::WRITE);
        }
    }
    return 0;
}

int IOStream::writeBytes(const char* data, int len, WriteCallback&& callback)
{
    assert(!write_callback_);

    write_callback_ = callback;
    int ret = writeBytes(data, len);
    if(ret)
    {
        write_callback_ = nullptr;
    }
    return ret;
}


int IOStream::justWriteBytesToBuff(const char* data, int len)
{
    if(closed_)
        return -1;
    if(len <= 0)
        return -2;

    if(false == write_buffer_.append(data, len) )
    { 
        TORNADO_LOG_ERROR_STR("Reached maximum read buffer size");
        this->close();
        return -3;
    }
    return 0;
}

int IOStream::readBytes(int num_bytes, ReadCallback&& callback)
{ 
    if(num_bytes <= 0 )
        return -1;
    assert(!read_callback_);
    assert(read_delimiter_.empty() && read_bytes_ == 0 && read_until_close_ == false );


    //本地缓存有数据，直接返回
    if (read_buffer_.readableBytes() >= (unsigned)num_bytes )
    {
        StringPtr result(new std::string);
        if( read_buffer_.readBytesToString(num_bytes, *result) )
        {
            if( callback )
            {   //read_callback_
                TORNADO_LOG_INFO_STR("local have data run read_callback_ put to ioloop callback");
                io_loop_->addCallback( std::bind(&inner_read_callback, callback, result) );
            }
            return 0;
        }
        TORNADO_LOG_ERROR_STR("readBytesToString failed");
        return 1;
    }


    read_bytes_ = num_bytes;
    read_callback_ = callback;

    if( handleRead())
    {
        TORNADO_LOG_ERROR_STR("handleRead failed");
        this->close();
        return -2;
    }

    if(!closed_)
    {
        addIOState(IOLoop::READ);
    }
    return 0;
}

int IOStream::readUntil(const std::string& delimiter, ReadCallback&& callback)
{
    if(delimiter.empty())
        return -1;

    assert(!read_callback_);
    assert(read_delimiter_.empty() && read_bytes_ == 0 && read_until_close_ == false );

    //本地缓存有数据，直接返回
    int index = read_buffer_.find(delimiter);
    if ( index >= 0 )
    {
        int num_bytes = index + delimiter.size();
        StringPtr result(new std::string);
        if( read_buffer_.readBytesToString(num_bytes, *result) )
        {
            if( callback )
            {   //read_callback_
                TORNADO_LOG_INFO_STR("local have data run read_callback_ put to ioloop callback");
                io_loop_->addCallback( std::bind(&inner_read_callback, callback, result) );
            }
            return 0;
        }
        TORNADO_LOG_ERROR_STR("readBytesToString failed");
        return 1;
    }

    read_delimiter_ = delimiter;
    read_callback_ = callback;

    if( handleRead() )
    {
        TORNADO_LOG_ERROR_STR("handleRead failed");
        this->close();
        return -2;
    }

    if(!closed_)
    {
        addIOState(IOLoop::READ);
    }
    return 0;
}
    
int IOStream::readUntilClose(ReadCallback&& callback)
{
    assert(!read_callback_);
    assert(read_delimiter_.empty() && read_bytes_ == 0 && read_until_close_ == false );
    
    read_until_close_ = true;
    read_callback_ = callback;

    if( handleRead() )
    {
        TORNADO_LOG_ERROR_STR("handleRead failed");
        this->close();
        return -1;
    }

    if(!closed_)
    {
        addIOState(IOLoop::READ);
    }
    return 0;
}

void IOStream::addIOState(int state)
{
    if(closed_)
    {
        TORNADO_LOG_ERROR_STR("stream have closed");
        return ;
    }
    if(state_ == 0)
    {
        state_ = IOLoop::ERROR | state;
        io_loop_->addHandler(socket_, 
              std::bind(&IOStream::handleEvents, shared_from_this(), std::placeholders::_1, std::placeholders::_2), 
              state_);
    }
    else
    {
        state_ |= state;
        io_loop_->updateHandler(socket_, state_);
    }
}
   

void IOStream::handleEvents(int32_t fd, uint32_t events)
{
    TORNADO_LOG_DEBUG("fd %d|events %0x0x", fd, events);
    if(closed_)
    {
        TORNADO_LOG_WARN("Got events for closed stream %d", fd);
        return ;
    }

    if(connecting_)
    {
        handleConnect();
    }

    if(closed_)
    {
        return ;
    }

    if( events & IOLoop::READ )
    {
        if( handleRead())
        {
            TORNADO_LOG_WARN("handleRead fail close fd=%d", fd);
            this->close();
            return ;
        }
    }
    if(closed_)
    {
        return ;
    }

    if( events & IOLoop::WRITE )
    {
        if( handleWrite())
        {
            TORNADO_LOG_WARN("handleWrite fail close fd=%d", fd);
            this->close();
            return ;
        }
    }
    if(closed_)
    {
        return ;
    }

    if( events & IOLoop::ERROR )
    {
        TORNADO_LOG_WARN("IOLoop::ERROR event close fd=%d", fd);
        this->close();
        return ;
    }
   
    uint32_t  state = IOLoop::ERROR;
    if( reading() )
    {
        state |= IOLoop::READ;
    }
    if( writing() )
    {
        state |= IOLoop::WRITE;
    }
    
    //if( state == IOLoop::ERROR && read_buffer_.readableBytes() == 0 )
    //{
    //    //# If the connection is idle, listen for reads too so
    //    //# we can tell if the connection is closed.  If there is
    //    //# data in the read buffer we won't run the close callback
    //    //# yet anyway, so we don't need to listen in this case.
    //    state |= IOLoop::READ;
    //    LOG_DEBUG("fd %d|add read events", fd);
    //}
    if( state == IOLoop::ERROR )
    {
        state |= IOLoop::READ;
    }

    if( state != state_ )
    {
        state_ = state;
        io_loop_->updateHandler(socket_, state_);
    }
}

void IOStream::handleConnect()
{
    if(SocketUtil::getSocketError(socket_))
    {
        TORNADO_LOG_ERROR("connnect failed strerrno:%s", STR_ERRNO);
        if(connect_callback_)
        {
            TORNADO_LOG_INFO_STR("run connect_callback_ put to ioloop callback");
            io_loop_->addCallback( std::bind(&inner_connect_callback,  connect_callback_, errno) );
            connect_callback_ = nullptr;
        }
        this->close();
        return ;
    }
    if( connect_callback_ )
    {
        TORNADO_LOG_INFO_STR("run connect_callback_ put to ioloop callback");
        io_loop_->addCallback( std::bind(&inner_connect_callback,  connect_callback_, 0) );
        connect_callback_ = nullptr;
    }
    connecting_ = false;
    TORNADO_LOG_INFO_STR("connnect succ");
}

bool IOStream::reading()
{
    return read_bytes_ > 0;
}
bool IOStream::writing()
{
    return write_buffer_.readableBytes() > 0;
}


int IOStream::handleRead()
{
    int ret = readToBufferLoop();
    if(ret)
    {
        return -1;
    }
    TORNADO_LOG_DEBUG("read_bytes_ %zu readbytes: %zu", read_bytes_, read_buffer_.readableBytes());
    if (read_bytes_ > 0 && read_buffer_.readableBytes() >= read_bytes_)
    {
        StringPtr result(new std::string);
        if( read_buffer_.readBytesToString(read_bytes_, *result) )
        {
            read_bytes_ = 0;
            if( read_callback_ )
            {
                TORNADO_LOG_INFO_STR("recv data run read_callback_ put to ioloop callback");
                io_loop_->addCallback( std::bind(&inner_read_callback, read_callback_, result) );
                read_callback_ = nullptr; 
            }
            return 0;
        }
        TORNADO_LOG_ERROR_STR("-------------- readBytesToString faile");
        return -1;
    }
    else if( !read_delimiter_.empty() )
    {
        int index = read_buffer_.find(read_delimiter_);
        if ( index >= 0 )
        {
            int num_bytes = index + read_delimiter_.size();
            StringPtr result(new std::string);
            if( read_buffer_.readBytesToString(num_bytes, *result) )
            {
                read_delimiter_.clear();
                if( read_callback_ )
                {
                    TORNADO_LOG_INFO_STR("recv data run read_callback_ put to ioloop callback");
                    io_loop_->addCallback( std::bind(&inner_read_callback, read_callback_, result) );
                    read_callback_ = nullptr;
                }
                return 0;
            }
            TORNADO_LOG_ERROR_STR("-------------- readBytesToString faile");
            return 1;
        }
    }
    else if(read_until_close_)
    {
        size_t buffsize = read_buffer_.readableBytes();
        if(buffsize > 0)
        {
            StringPtr result(new std::string);
            if( read_buffer_.readBytesToString(buffsize, *result) )
            {
                if( read_callback_ )
                {
                    TORNADO_LOG_INFO_STR("recv data run read_callback_ put to ioloop callback");
                    io_loop_->addCallback( std::bind(&inner_read_callback, read_callback_, result) );
                }
                else
                {
                    TORNADO_LOG_WARN_STR("read_until_close_ but read_callback_ is emtpy");
                }
                return 0;
            }
            TORNADO_LOG_ERROR_STR("-------------- readBytesToString faile");
            return -1;
        }    
    }
    return 0;
}


int IOStream::readToBufferLoop()
{
    while(true)
    {
        int ret = readToBuffer();
        if(ret == 0)
            break;
        if(ret < 0)
            return -1;

        //QQQ should check ???
        // If we've reached target_bytes, we know we're done.
        //if (read_bytes_ > 0 && read_buffer_.readableBytes() >= read_bytes_)
        //    break;


        /*if( !read_delimiter_.empty() )
        {
            int index = read_buffer_.find(read_delimiter_);
            if ( index >= 0 )
                break;
        }*/
    }
    return 0;
}



int IOStream::readToBuffer()
{
    if(!read_buffer_.ensureWritableBytes( READ_CHUNK_SIZE ) )
    {
        TORNADO_LOG_ERROR_STR("read_buffer_ ensureWritableBytes failed Reached maximum read buffer size");
        return -1;
    }

    int bytesize = read(socket_, read_buffer_.data(), READ_CHUNK_SIZE);

    TORNADO_LOG_DEBUG("try read and read bytesize %d", bytesize);
    
    if( bytesize == 0 )
    {
        TORNADO_LOG_WARN_STR("read bytesize is 0");
        //close out 
        return -2;
    }
    
    if(bytesize < 0)
    {
        if(ERRNO_WOULDBLOCK)
            return 0;
        TORNADO_LOG_WARN("read bytesize  %d  < 0 strerron:%s", bytesize, STR_ERRNO);
        //close out 
        return -3;
    }
  
    if(false == read_buffer_.extendReadBytes(bytesize) )
    {
        TORNADO_LOG_ERROR_STR("Reached maximum read buffer size");
        return -4;
    }

    return bytesize;
}



int IOStream::handleWrite()
{
    TORNADO_LOG_DEBUG_STR("");
    size_t  writebytes = write_buffer_.readableBytes();
    while( writebytes > 0 )
    {
        int chunk_size = std::min( (size_t)WRITE_CHUNK_SIZE, writebytes );
        int num_bytes = write(socket_, write_buffer_.data(), chunk_size ); 
        TORNADO_LOG_DEBUG("write num_bytes=%d", num_bytes);
        if (num_bytes > 0)
        {
            if( !write_buffer_.readBytes( num_bytes ) )
                return -1;

            writebytes = write_buffer_.readableBytes();
        }
        else if(num_bytes == 0)
        {
            break;//nothing was written
        }
        else //error
        {
            if( ERRNO_WOULDBLOCK )
                break;
                
            if (!ERRNO_CONNRESET)
            {
                //# Broken pipe errors are usually caused by connection
                //    # reset, and its better to not log EPIPE errors to
                //    # minimize log spam
                //    gen_log.warning("Write error on %s: %s",
                //                    self.fileno(), e)
            }
            TORNADO_LOG_WARN("write writebytes %d < 0 strerron:%s", num_bytes, STR_ERRNO);
            return -1;
        }
    }
    if( write_buffer_.readableBytes() == 0 )
    {
        if(write_callback_)
        {
            TORNADO_LOG_DEBUG_STR("write done run write_callback_ on ioloop callback");
            io_loop_->addCallback( std::move(write_callback_) );
            write_callback_ = nullptr;//release
        }
    }
    return 0;
}
            
}//namespace 



