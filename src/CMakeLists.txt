cmake_minimum_required(VERSION 2.6)
project(dhcpext)

add_subdirectory(pc)
find_package(Threads)

add_executable(dhcpext common.c dhcplease.c main.c proto.c rp.c rx.c stream.c tx.c)
target_link_libraries(dhcpext pc ${CMAKE_THREAD_LIBS_INIT})

INSTALL(TARGETS dhcpext RUNTIME DESTINATION /usr/bin/)
