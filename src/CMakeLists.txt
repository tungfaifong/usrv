# Minimum CMake required
cmake_minimum_required(VERSION 3.10.2)

set(usrv_header
	./common/buffer.h
	./common/common_def.h
	./common/object_pool.h
	./common/path.h
	./common/swap_list.h
	./common/time.h
	./network/message.h
	./network/tcp_message.h
	./network/tcp_server.h
	./network/tcp_server_impl.h
	./rpc/rpc_message.h
	./rpc/rpc_server.h
	./rpc/rpc_server_impl.h
	./script/lua_manager.h
	./script/lua_manager_impl.h
	./timer/timer_manager.h
	./timer/timer_manager_impl.h
	./unit.h
	./unit_manager.h
	./unit_manager_impl.h
)

set(usrv_files
	./common/time.cpp
	./network/message.cpp
	./network/tcp_message.cpp
	./network/tcp_server.cpp
	./rpc/rpc_message.cpp
	./rpc/rpc_server.cpp
	./script/lua_manager.cpp
	./timer/timer_manager.cpp
	./unit_manager.cpp
)

include_directories(
${PROJECT_SOURCE_DIR}/src
${INCLUDE_PATH}
)

add_library(usrv ${usrv_header} ${usrv_files})

install(TARGETS usrv
ARCHIVE DESTINATION lib
)

set(usrv_include
	./common/common_def.h
	./common/path.h
	./common/time.h
	./network/tcp_server.h
	./rpc/rpc_server.h
	./script/lua_manager.h
	./timer/timer_manager.h
	./unit.h
	./unit_manager.h
)

foreach(file ${usrv_include})
	string(REPLACE "\\" "/" _header ${file})
	get_filename_component(_extract_to ${_header} PATH)
	install(FILES ${file} DESTINATION include/${_extract_to})
endforeach()
