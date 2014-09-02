add_library(rx rx.c receive.c)

target_link_libraries(rx common stream)
