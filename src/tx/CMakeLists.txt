add_library(tx tx.c transmit.c config.c commands.c)

target_link_libraries(tx common proto stream)
