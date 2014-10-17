简单说明
====================
###　　Date: 2014-10-16
###　　EMail: 979762787@qq.com

===========================

基于[tornado](https://github.com/tornadoweb/tornado) 实现的一个c++异步库（依赖boost库， gcc >= 4.4.5）

##文件说明

tornado/
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
|-- TcpServer.cpp
|-- TcpServer.h
|-- TimeoutQueue.cpp
|-- TimeoutQueue.h
|-- Util.h
|-- Web.cpp
|-- Web.h
`-- build


build 编译目录
```Bash
cd build
cmake ..
make
```
编译后lib库  lib/libtornado.a
需要修改CMakeLists.txt里面boost 路径 SET(BOOST ...) 


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


