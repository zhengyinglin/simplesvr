简单说明
====================
###　　Date: 2014-11-25
###　　EMail: 979762787@qq.com

===========================

基于[tornado](https://github.com/tornadoweb/tornado) 实现的一个c++异步库(需要c++11才可以编译)
（
依赖库:
boost_1_57_0
gflags-2.1.1
c-ares-1.10.0  (for DNS) 
）

##文件说明

tornado/
|-- App.cpp
|-- App.h
|-- Buffer.cpp
|-- Buffer.h
|-- CMakeLists.txt
|-- EPoll.cpp
|-- EPoll.h
|-- HttpServer.cpp
|-- HttpServer.h
|-- IOLoop.cpp
|-- IOLoop.h
|-- IOStream.cpp
|-- IOStream.h
|-- Logging.cpp
|-- Logging.h
|-- ReadMe.md
|-- Resolver.cpp
|-- Resolver.h
|-- TcpServer.cpp
|-- TcpServer.h
|-- TimeoutQueue.cpp
|-- TimeoutQueue.h
|-- Util.h
|-- Web.cpp
|-- Web.h
|-- build
|-- shm
|   |-- container.hpp
|   `-- mq.hpp
`-- util
    |-- boost_ptree.h
    |-- comm.cpp
    |-- comm.h
    |-- file.h
    |-- ini.h
    |-- json.h
    |-- random.h
    |-- scople_guard.h
    `-- xml.h


build 编译目录
```Bash
cd build
cmake ..
make
```
编译后lib库  lib/libtornado.a
需要修改CMakeLists.txt里面boost、gflag c-ares 路径 SET  BOOST  GFLAGS CARES


EPoll.h   epoll 简单封装
IOLoop.h  事件主循环
Buffer.h  一个缓存队列
IOStream.h  异步socket封装
TimeoutQueue.h  定时器队列
Logging.h       日志简单封装
Util.h          常用函数封装
TcpServer.h      简单tcp 服务器
HttpServer.h     基于tcpsvr的一个http服务器
Web.h            httpsvr的封装
Resolver.h       异步DNS封装（gethostbyname）


##编译gflag
https://github.com/schuhschuh/gflags

tar -xvf gflags-2.1.1.tar.gz
cd gflags-2.1.1 && mkdir build && cd build
CXXFLAGS="-fPIC -O3 -DNDEBUG"  cmake .. -DGFLAGS_NAMESPACE=google  -DCMAKE_INSTALL_PREFIX=/xxxx/google/gflags-2.1.1 
make VERBOSE=1
make install


##编译c-ares
https://github.com/schuhschuh/gflags

tar xvf c-ares-1.10.0.tar.gz 
cd c-ares-1.10.0
./configure --prefix=/xxxx/c-ares
make 
make install

##boost
不用编译，只使用boost的头文件

