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
./tcp_svr --logfile=svrlog.log  --port=7777  --loglevel=3


###客户端

在机器 10.193.0.103 启动10个客户端
python  start_tcp_client.py 

```Python
#start_tcp_client.py 
import os
cmd = './tcp_client --ip=10.193.0.102 --port=7777 --pkgnum=50000 --loglevel=3 --logfile=tcp_log_%s  &'
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

  36730 1414064535
  63343 1414064536
  63127 1414064537
  63119 1414064538
  62808 1414064539
  63024 1414064540
  63176 1414064541
  60577 1414064542
  24096 1414064543
```

大概每秒能够处理 63K 的请求



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
Elapsed time:                   6.46 secs
Data transferred:               0.19 MB
Response time:                  0.01 secs
Transaction rate:           15479.88 trans/sec
Throughput:                     0.03 MB/sec
Concurrency:                   99.34
Successful transactions:      100000
Failed transactions:               0
Longest transaction:            0.02
Shortest transaction:           0.00
```

大概每秒 15K多 请求



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
Elapsed time:                   7.54 secs
Data transferred:               1.05 MB
Response time:                  0.01 secs
Transaction rate:           13262.60 trans/sec
Throughput:                     0.14 MB/sec
Concurrency:                   99.68
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
Elapsed time:                  14.13 secs
Data transferred:              39.67 MB
Response time:                  0.01 secs
Transaction rate:            7077.14 trans/sec
Throughput:                     2.81 MB/sec
Concurrency:                   99.66
Successful transactions:      100000
Failed transactions:               0
Longest transaction:            0.02
Shortest transaction:           0.00
```

大概每秒 7k多（之前的1半） 请求，如果tcp请求使用连接池可能性能会好一点


