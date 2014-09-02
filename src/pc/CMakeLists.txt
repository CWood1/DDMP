add_library(pc api.c heartbeat.c pc.c response.c)

target_link_libraries(pc common proto stream)
