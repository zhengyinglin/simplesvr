简单说明
====================
###　　Date: 2014-11-20
###　　EMail: 979762787@qq.com

===========================



##文件说明

test/
|-- CMakeLists.txt
|-- build
|-- gen_http_code.py
|-- http_client.py
|-- http_svr.cpp
|-- http_svr_2.cpp
|-- tcp_client.cpp
|-- tcp_svr.cpp
`-- ReadMe.md

tcp_client.cpp、tcp_svr.cpp 简单tcp的测试代码  
发送400字节给服务器，服务器回包收到的400字节给客户端

http_svr.cpp、http_svr_2.cpp 简单http的测试代码  

build 编译目录(需要提前编译好libtornado.a)
```Bash
cd build
cmake ..
make
```
编译后bin文件在  bin/
bin/
|-- http_svr
|-- http_svr_2
|-- tcp_client
`-- tcp_svr

##TCP 简单测试

```Text
机器1              机器2
tcp_svr   < -- >   tcp_client
```

###服务器

在机器上10.193.0.102启动
./tcp_svr --logfile=svrlog.log  --port=7777  --loglevel=3

注意如果用clang 编译出来的会依赖高版本gcc的2个库 lib64/libstdc++.so.6  libgcc_s.so.1 

###客户端

在机器 10.193.0.103 启动10个客户端
python  start_tcp_client.py 

```Python
#start_tcp_client.py 
import os
cmd = './tcp_client --ip=10.193.0.102 --port=7777 --pkgnum=60000 --loglevel=3 --logfile=tcp_log_%s  &'
n = 10 
for i in xrange(n):
    os.system(cmd % i)
```


###简单日志统计
```Bash
日志内容
tcp_log_0:INFO|1414064535.406|tcp_client.cpp|101|readDone|PKG Done
tcp_log_0:INFO|1414064535.406|tcp_client.cpp|101|readDone|PKG Done
tcp_log_0:INFO|1414064535.407|tcp_client.cpp|101|readDone|PKG Done
tcp_log_0:INFO|1414064535.407|tcp_client.cpp|101|readDone|PKG Done
tcp_log_0:INFO|1414064535.407|tcp_client.cpp|101|readDone|PKG Done
tcp_log_0:INFO|1414064535.407|tcp_client.cpp|101|readDone|PKG Done
tcp_log_0:INFO|1414064535.407|tcp_client.cpp|101|readDone|PKG Done

grep "PKG Done" tcp_log_* | sort -t\| -k2 |  cut -c 16-25 | uniq -c

  49732 1416484097
  67581 1416484098
  66634 1416484099
  67322 1416484100
  66882 1416484101
  66831 1416484102
  66862 1416484103
  66873 1416484104
  64821 1416484105
  16462 1416484106 
```

大概每秒能够处理 67K 的请求



##简单http测试
客户端发请求给服务器 --> http服务器回“ok”给客户端

###服务器
在10.193.0.103 上  
./http_svr --logfile=log_http.log --port=8888  --loglevel=3

###客户端
在10.193.0.102 上

100个连接  连续发送1000个连接
```Bash
./siege -b -c 100 -r 1000 http://10.193.0.103:8888/  > httplog.log

** SIEGE 3.0.0
** Preparing 100 concurrent users for battle.
The server is now under siege...
done.

Transactions:                 100000 hits
Availability:                 100.00 %
Elapsed time:                   6.25 secs
Data transferred:               0.19 MB
Response time:                  0.01 secs
Transaction rate:           16000.00 trans/sec
Throughput:                     0.03 MB/sec
Concurrency:                   99.35
Successful transactions:      100000
Failed transactions:               0
Longest transaction:            0.02
Shortest transaction:           0.00
```

大概每秒 16K 请求



##简单http_2测试1
客户端发请求给服务器 --> http服务器回“ok”给客户端

###服务器
在10.193.0.103 上
./http_svr_2  --logfile=log_http_2.log --port=8888  --loglevel=3

###客户端
在10.193.0.102 上
100个连接  连续发送1000个连接
```Bash
./siege -b -c 100 -r 1000 http://10.193.0.103:8888/  > httplog.log
** SIEGE 3.0.0
** Preparing 100 concurrent users for battle.
The server is now under siege...
done.

Transactions:                 100000 hits
Availability:                 100.00 %
Elapsed time:                   7.16 secs
Data transferred:               1.05 MB
Response time:                  0.01 secs
Transaction rate:           13966.48 trans/sec
Throughput:                     0.15 MB/sec
Concurrency:                   99.52
Successful transactions:      100000
Failed transactions:               0
Longest transaction:            0.02
Shortest transaction:           0.00
```
大概每秒 13K多 请求

##简单http_2测试2

客户端发请求给服务器 --> http服务器，发送400字节 --> tcp 服务器 
       回ok给客户端 <--  收到tcp的400字节        <-- 


###服务器
在10.193.0.103 上
./http_svr_2  --logfile=log_http_2.log  --port=8888 --tcp_ip=10.193.0.102 --tcp_port=7777  --loglevel=3
在10.193.0.102 上
./tcp_svr  --logfile=logtcp.log  --port=7777  --loglevel=3

###客户端
在10.193.0.102 上
100个连接  连续发送1000个连接

```Bash
./siege -b -c 100 -r 1000 http://10.193.0.103:8888/example  > httplog


** SIEGE 3.0.0
** Preparing 100 concurrent users for battle.
The server is now under siege...
done.

Transactions:                 100000 hits
Availability:                 100.00 %
Elapsed time:                  13.10 secs
Data transferred:              39.67 MB
Response time:                  0.01 secs
Transaction rate:            7633.59 trans/sec
Throughput:                     3.03 MB/sec
Concurrency:                   99.62
Successful transactions:      100000
Failed transactions:               0
Longest transaction:            0.03
Shortest transaction:           0.00
```

大概每秒 7k多（之前的1半） 请求，如果tcp请求使用连接池可能性能会好一点



##与nginix对比
对于http请求，单进程极限的 13K/s 左右，相比nginix（21K/s）还是有一点差距

nginx helloworld 处理数据
https://github.com/perusio/nginx-hello-world-module
```Bash
./siege -b -c 100 -r 1000 http://10.193.0.103:8080/hello  > httplog.log
** SIEGE 3.0.0
** Preparing 100 concurrent users for battle.
The server is now under siege...
done.

Transactions:                 100000 hits
Availability:                 100.00 %
Elapsed time:                   4.63 secs
Data transferred:               1.14 MB
Response time:                  0.00 secs
Transaction rate:           21598.27 trans/sec
Throughput:                     0.25 MB/sec
Concurrency:                   99.20
Successful transactions:      100000
Failed transactions:               0
Longest transaction:            0.02
Shortest transaction:           0.00

./siege -b -c 300 -r 1500 http://10.193.0.103:8080/hello  > httplog.log
** SIEGE 3.0.0
** Preparing 300 concurrent users for battle.
The server is now under siege...
done.

Transactions:                 450000 hits
Availability:                 100.00 %
Elapsed time:                  20.91 secs
Data transferred:               5.15 MB
Response time:                  0.01 secs
Transaction rate:           21520.80 trans/sec
Throughput:                     0.25 MB/sec
Concurrency:                  298.76
Successful transactions:      450000
Failed transactions:               0
Longest transaction:            0.04
Shortest transaction:           0.00
```