add_library(rp rp.c heartbeat.c commands.c)

target_link_libraries(rp common proto)
