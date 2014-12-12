CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(SERVER_NAME svr)

PROJECT(${SERVER_NAME})


set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wall -g -std=c++11 -O2")

#remove boost sth warning for clang building
set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedef")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

SET(BOOST    /yourpath/boost/boost_1_54_0/)
SET(TORNADO  /yourpath/somelib/svr/)
SET(GFLAGS   /yourpath/lib/google/gflags-2.1.1)
SET(CARES    /yourpath/lib/c-ares)

INCLUDE_DIRECTORIES(
       ${BOOST}
	   ${TORNADO}
	   ${GFLAGS}/include
	   ${CARES}/include
	)


ADD_EXECUTABLE(test_dns
					test_dns
			   )


TARGET_LINK_LIBRARIES(test_dns
	${TORNADO}/tornado/build/lib/libtornado.a
	${GFLAGS}/lib/libgflags_nothreads.a
	pthread  rt
	${CARES}/lib/libcares.a
)



ADD_EXECUTABLE(tcp_svr
					tcp_svr
			   )

TARGET_LINK_LIBRARIES(tcp_svr
	${TORNADO}/tornado/build/lib/libtornado.a
	${GFLAGS}/lib/libgflags_nothreads.a
	pthread
)



ADD_EXECUTABLE(tcp_client
					tcp_client
			   )


TARGET_LINK_LIBRARIES(tcp_client
	${TORNADO}/tornado/build/lib/libtornado.a
	${GFLAGS}/lib/libgflags_nothreads.a
	pthread
)



ADD_EXECUTABLE(http_svr
					http_svr
			   )


TARGET_LINK_LIBRARIES(http_svr
	${TORNADO}/tornado/build/lib/libtornado.a
	${GFLAGS}/lib/libgflags_nothreads.a
	pthread
)



ADD_EXECUTABLE(http_svr_2
					http_svr_2
			   )


TARGET_LINK_LIBRARIES(http_svr_2
	${TORNADO}/tornado/build/lib/libtornado.a
	${GFLAGS}/lib/libgflags_nothreads.a
	pthread
)



ADD_EXECUTABLE(test_shm
					test_shm
			   )

TARGET_LINK_LIBRARIES(test_shm
	${TORNADO}/tornado/build/lib/libtornado.a
	pthread 
	rt
)

ADD_EXECUTABLE(test_util
					test_util
					${TORNADO}/tornado/util/comm
			   )

TARGET_LINK_LIBRARIES(test_util
	${TORNADO}/tornado/build/lib/libtornado.a
)