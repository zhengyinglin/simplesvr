CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(tornado)

#编译选项
set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wall -g -std=c++0x -O2")

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(BOOST    /home/ghostzheng/quiz/trunk/ext/boost/boost_1_54_0/)

#源文件
AUX_SOURCE_DIRECTORY(. APP_SRC_FILES)



#include目录
INCLUDE_DIRECTORIES(
	 ${BOOST}
	)

ADD_LIBRARY(tornado STATIC   ${APP_SRC_FILES})

