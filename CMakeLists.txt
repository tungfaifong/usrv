# Minimum CMake required
cmake_minimum_required(VERSION 3.10.2)

project (usrv)

set(CMAKE_CXX_FLAGS_RELEASE "/MT")
set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -DASIO_STANDALONE -pthread")

#dependence         include-path        lib-path
#asio-1.12.2        √
#lua-5.2.0          √                   √
#LuaBridge-2.4.1    √
#protobuf-3.11.3    √                   √
#spdlog-1.5.0       √
set(3RDPARTY_PATH ${PROJECT_SOURCE_DIR}/3rdparty)
set(ASIO_INCLUDE_PATH ${3RDPARTY_PATH}/asio-1.12.2/inc)
set(LUA_INCLUDE_PATH ${3RDPARTY_PATH}/lua-5.2.0/inc)
set(LUA_BRIDGE_INCLUDE_PATH ${3RDPARTY_PATH}/LuaBridge-2.4.1/inc)
set(PROTOBUF_INCLUDE_PATH ${3RDPARTY_PATH}/protobuf-2.4.1/inc)
set(SPDLOG_INCLUDE_PATH ${3RDPARTY_PATH}/spdlog-1.5.0/inc)
set(INCLUDE_PATH ${ASIO_INCLUDE_PATH}
    ${LUA_INCLUDE_PATH}
    ${LUA_BRIDGE_INCLUDE_PATH}
    ${PROTOBUF_INCLUDE_PATH}
    ${SPDLOG_INCLUDE_PATH}
    )
set(LUA_LIB_PATH ${3RDPARTY_PATH}/lua-5.2.0/lib)
set(PROTOBUF_LIB_PATH ${3RDPARTY_PATH}/protobuf-3.11.3/lib)
set(LIB_PATH ${LUA_LIB_PATH}
    ${PROTOBUF_LIB_PATH}
    )

add_subdirectory(src)
