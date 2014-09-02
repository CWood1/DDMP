cmake_minimum_required(VERSION 2.6)
project(dhcpext)

if(UNIX)
	if(CMAKE_COMPILER_IS_GNUCC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
	endif(CMAKE_COMPILER_IS_GNUCC)
endif(UNIX)

include_directories(include)

add_subdirectory(libs)

add_subdirectory(tx)
add_subdirectory(rx)
add_subdirectory(pc)
add_subdirectory(rp)

find_package(Threads)

add_executable(dhcpext dhcplease.c main.c)
target_link_libraries(dhcpext common proto tx rx pc rp ${CMAKE_THREAD_LIBS_INIT})

INSTALL(TARGETS dhcpext RUNTIME DESTINATION /usr/bin/)
