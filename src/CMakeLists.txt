cmake_minimum_required(VERSION 3.0)
project(ddmpd)

if(UNIX)
	if(CMAKE_COMPILER_IS_GNUCC)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -pedantic \
			-Wshadow -Wpointer-arith -Wcast-align -Wwrite-strings -Wmissing-prototypes \
			-Wmissing-declarations -Wredundant-decls -Wnested-externs -Winline -Wno-long-long \
			-Wuninitialized -Wconversion -Wstrict-prototypes")
	endif(CMAKE_COMPILER_IS_GNUCC)
endif(UNIX)

include_directories(include)

add_subdirectory(libs)

add_subdirectory(tx)
add_subdirectory(rx)
add_subdirectory(pc)
add_subdirectory(rp)

find_package(Threads)

add_executable(ddmpd main.c)
target_link_libraries(ddmpd common proto tx rx pc rp ${CMAKE_THREAD_LIBS_INIT})

INSTALL(TARGETS ddmpd RUNTIME DESTINATION /usr/bin/)
