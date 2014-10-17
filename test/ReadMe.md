简单说明
====================
###　　Date: 2014-10-16
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
./tcp_svr INFO > svrlog.log


###客户端

在机器 10.193.0.103 启动10个客户端
python  start_tcp_client.py 

```Python
#start_tcp_client.py 
import os
cmd = './tcp_client 10.193.0.102 7777 50000 INFO > tcp_log_%s  &'
n = 10 
for i in xrange(n):
    os.system(cmd % i)
```

###简单日志统计
```Bash
日志内容
tcp_log_0:INFO|1413440828.751|/home/ghostzheng/somelib/svr/tcp_client.cpp|95|readDone|""
tcp_log_0:INFO|1413440828.751|/home/ghostzheng/somelib/svr/tcp_client.cpp|95|readDone|""
tcp_log_0:INFO|1413440828.751|/home/ghostzheng/somelib/svr/tcp_client.cpp|95|readDone|""
tcp_log_0:INFO|1413440828.751|/home/ghostzheng/somelib/svr/tcp_client.cpp|95|readDone|""

grep readDone  tcp_log* | sort -t\| -k2 | cut -c 16-25 | uniq -c

  15192 1413441028
  54136 1413441029
  54290 1413441030
  54108 1413441031
  53525 1413441032
  54047 1413441033
  53978 1413441034
  54396 1413441035
  53559 1413441036
  47053 1413441037
   5716 1413441038
```

大概每秒能够处理 5w多的请求



##简单http测试
客户端发请求给服务器 --> http服务器回“ok”给客户端

###服务器
在10.193.0.103 上
./http_svr INFO > log_http.log

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
Elapsed time:                   7.27 secs
Data transferred:               0.19 MB
Response time:                  0.01 secs
Transaction rate:           13755.16 trans/sec
Throughput:                     0.03 MB/sec
Concurrency:                   99.50
Successful transactions:      100000
Failed transactions:               0
Longest transaction:            0.02
Shortest transaction:           0.00
```

大概每秒 1w多 请求



##简单http_2测试1
客户端发请求给服务器 --> http服务器回“ok”给客户端

###服务器
在10.193.0.103 上
./http_svr_2 INFO > log_http_2.log

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
Elapsed time:                   8.16 secs
Data transferred:               1.05 MB
Response time:                  0.01 secs
Transaction rate:           12254.90 trans/sec
Throughput:                     0.13 MB/sec
Concurrency:                   99.54
Successful transactions:      100000
Failed transactions:               0
Longest transaction:            0.02
Shortest transaction:           0.00
```
大概每秒 1w多 请求

##简单http_2测试2

客户端发请求给服务器 --> http服务器，发送400字节 --> tcp 服务器 
       回ok给客户端 <--  收到tcp的400字节        <-- 


###服务器
在10.193.0.103 上
./http_svr_2 INFO > log_http_2.log
在10.193.0.102 上
./tcp_svr INFO > logtcp.log 

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
Elapsed time:                  16.63 secs
Data transferred:              39.67 MB
Response time:                  0.02 secs
Transaction rate:            6013.23 trans/sec
Throughput:                     2.39 MB/sec
Concurrency:                   99.66
Successful transactions:      100000
Failed transactions:               0
Longest transaction:            0.03
Shortest transaction:           0.01
```

大概每秒 6k多（之前的1半） 请求，如果tcp请求使用连接池可能性能会好一点


