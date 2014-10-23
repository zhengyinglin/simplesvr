CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
SET(SERVER_NAME svr)

PROJECT(${SERVER_NAME})


set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} -Wall -g -std=c++0x -O2")


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

SET(BOOST    /yourpath/boost/boost_1_54_0/)
SET(TORNADO  /yourpath/somelib/svr/)
SET(GFLAGS   /yourpath/lib/google/gflags-2.1.1)


INCLUDE_DIRECTORIES(
       ${BOOST}
	   ${TORNADO}
	   ${GFLAGS}/include
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
					test_base
			   )

TARGET_LINK_LIBRARIES(test_shm
	${TORNADO}/tornado/build/lib/libtornado.a
	pthread 
	rt
)

