CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(tornado)


set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wall -g -std=c++11 -O2")

#remove boost sth warning for clang building
set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedef")

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(BOOST    /yourpath/boost/boost_1_54_0/)
SET(GFLAGS   /yourpath/lib/google/gflags-2.1.1)
SET(CARES   /yourpath/lib/c-ares)

AUX_SOURCE_DIRECTORY(. APP_SRC_FILES)


INCLUDE_DIRECTORIES(
	 ${BOOST}
	 ${GFLAGS}/include
	 ${CARES}/include
	)

ADD_LIBRARY(tornado STATIC   ${APP_SRC_FILES})

