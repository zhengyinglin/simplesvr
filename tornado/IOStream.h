/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-10-27
*/
#ifndef _TORNADO_IO_STREAM_H_
#define _TORNADO_IO_STREAM_H_
#include "IOLoop.h"
#include "Buffer.h"
#include "boost/enable_shared_from_this.hpp"
#include "boost/shared_ptr.hpp"

namespace tornado
{
class IOLoop;

class IOStream : public boost::enable_shared_from_this<IOStream>
{
public:
    typedef boost::function<void(const std::string&)> ReadCallback;
    typedef boost::function<void(int)> ConnectCallback;
    typedef IOLoop::Callback   WriteCallback;
    typedef IOLoop::Callback   CloseCallback;
    enum { 
        WRITE_CHUNK_SIZE = 128 * 1024  / 64,
        READ_CHUNK_SIZE =  64 * 1024  / 64,
    };

    IOStream(int32_t socket);
    ~IOStream();

    bool closed() const {return closed_;}

    int32_t getFd()const {return socket_;}

    void close();
    
    bool connect(const char* ip, short port);
    bool connect(const char* ip, short port, ConnectCallback callback);
    

    void setCloseCallback(CloseCallback callback);

  
    int writeBytes(const char* data, int len);
    int writeBytes(const char* data, int len, WriteCallback callback);
    int writeBytes(const std::string& data)
    {
        return writeBytes(data.c_str(), data.size());
    }
    int justWriteBytesToBuff(const char* data, int len);

    int readBytes(int num_bytes, ReadCallback callback);

    int readUntil(const std::string& delimiter, ReadCallback callback);

    bool reading();
    bool writing();
protected:
    void addIOState(int state);
    void handleEvents(int32_t fd, uint32_t events);
    void handleConnect();

    int  handleRead();
    int  readToBufferLoop();
    int  readToBuffer();
    int  handleWrite();

private:
    int32_t socket_;
    bool connecting_;
    bool closed_;
    uint32_t state_;
    IOLoop*  io_loop_;
    ConnectCallback  connect_callback_;
    ReadCallback     read_callback_;
    WriteCallback    write_callback_;
    CloseCallback    close_callback_;
    Buffer  read_buffer_;
    Buffer  write_buffer_;
    size_t   read_bytes_;
    std::string read_delimiter_;
};

typedef boost::shared_ptr<IOStream> IOStreamPtr;



}//namespace tornado
#endif //_TORNADO_IO_STREAM_H_