add_subdirectory(proto)
add_subdirectory(stream)

add_library(common common.c network.c)
target_link_libraries(common stream)
