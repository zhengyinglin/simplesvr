/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-11-21
*/
#ifndef _UTIL_COMM_FILE_H_
#define _UTIL_COMM_FILE_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>


namespace util
{

namespace comm
{

class File 
{
public:
  enum STATE {ERROR = -1, SUCC = 0, LOCKED = 1 };

  explicit File(const char* name = nullptr, int flags = O_RDONLY, mode_t mode = 0666)
  {
      fd_ = -1;
      if(name != nullptr)
      {
         fd_ = ::open(name, flags, mode);
      }
  }

  bool is_open() const {return fd_ != -1; }
  int fd() const { return fd_; }

  virtual ~File()
  {
      this->close();
  }

  void close() 
  {
      if(fd_ != -1 )
      {
          ::close(fd_);
          fd_ = -1;
      }
  }

  // movable
  File(File&& other):
      fd_(other.fd_)
  {
      other.close();
  }
  File& operator=(File&& other) 
  {
      if(&other != this)
      {
          this->close();
          fd_  = other.fd_;
          other.fd_ = -1;
      }
      return *this;
  }

  // 文件锁，进程间同步使用
  inline int lock() 
  {
      return doLock(LOCK_EX);
  }
  inline int try_lock() 
  { 
      return doTryLock(LOCK_EX); 
  }
  inline int unlock()
  {
      return doLock(LOCK_UN);
  }

  inline int lock_shared()
  { 
      return doLock(LOCK_SH);
  }
  inline int try_lock_shared(){ return doTryLock(LOCK_SH); }
  inline int unlock_shared()  { return unlock(); }

private:
  int doLock(int op)
  {
      int r = 0;
      do 
      {
          r = flock(fd_, op);
      } while (r == -1 && errno == EINTR);

      if(r)
          return ERROR;
      return SUCC;
  }

  int doTryLock(int op)
  {
      op |=  LOCK_NB;
      int r = 0;
      do 
      {
          r = flock(fd_, op);
      } while (r == -1 && errno == EINTR);
    
      // flock returns EWOULDBLOCK if already locked
      if (r == -1 && errno == EWOULDBLOCK)
          return LOCKED;
      
      if(r)
          return ERROR;

      return SUCC;
  }

  // unique
  File(const File&) = delete;
  File& operator=(const File&) = delete;

  int fd_;
};



}//namespace comm

}//namespace util

#endif //_UTIL_COMM_FILE_H_



#include <future>