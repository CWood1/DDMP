add_library(pc api.c heartbeat.c pc.c response.c commands.c)

target_link_libraries(pc common proto)
