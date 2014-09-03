#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include <dhcpext/pc.h>
#include <dhcpext/proto.h>
#include <dhcpext/stream.h>

#include <netinet/in.h>

void removeHeartbeatFromList(lHeartbeat**, lHeartbeat*);
void freeHeartbeatList(lHeartbeat*);
void handleSentHeartbeat(lHeartbeat**, lHeartbeat*);
void handleReceivedHeartbeat(heartbeat*, struct in_addr, int);
int checkMatchedHeartbeat(lHeartbeat**, response*);
void removeTimedoutHeartbeats(lHeartbeat**);

#endif

