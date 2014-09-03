add_library(rx rx.c receive.c commands.c)

target_link_libraries(rx common stream)
