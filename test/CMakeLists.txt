CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(SERVER_NAME svr)

PROJECT(${SERVER_NAME})

#编译选项
set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wall -g -std=c++0x -O2")

#二进制文件存放目录  放在build/bin
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

SET(BOOST    /home/ghostzheng/quiz/trunk/ext/boost/boost_1_54_0/)
SET(TORNADO  /home/ghostzheng/somelib/svr/)

#include目录
INCLUDE_DIRECTORIES(
       ${BOOST}
	   ${TORNADO}
	)


#生成目标文件 服务器
ADD_EXECUTABLE(tcp_svr
					tcp_svr
			   )
#用到的库
TARGET_LINK_LIBRARIES(tcp_svr
	${TORNADO}/tornado/build/lib/libtornado.a
)


#生成目标文件 客户端
ADD_EXECUTABLE(tcp_client
					tcp_client
			   )

#用到的库
TARGET_LINK_LIBRARIES(tcp_client
	${TORNADO}/tornado/build/lib/libtornado.a
)


#生成目标文件 服务器
ADD_EXECUTABLE(http_svr
					http_svr
			   )

#用到的库
TARGET_LINK_LIBRARIES(http_svr
	${TORNADO}/tornado/build/lib/libtornado.a
)


#生成目标文件 服务器
ADD_EXECUTABLE(http_svr_2
					http_svr_2
			   )

#用到的库
TARGET_LINK_LIBRARIES(http_svr_2
	${TORNADO}/tornado/build/lib/libtornado.a
)

