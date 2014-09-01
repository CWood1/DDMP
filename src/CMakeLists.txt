cmake_minimum_required(VERSION 2.6)
project(dhcpext)

include_directories(include)

add_subdirectory(libs)

add_subdirectory(tx)
add_subdirectory(rx)
add_subdirectory(pc)
add_subdirectory(rp)

find_package(Threads)

add_executable(dhcpext common.c dhcplease.c main.c proto.c stream.c)
target_link_libraries(dhcpext common tx rx pc rp ${CMAKE_THREAD_LIBS_INIT})

INSTALL(TARGETS dhcpext RUNTIME DESTINATION /usr/bin/)
