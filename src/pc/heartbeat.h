#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include <dhcpext/pc.h>

#include "../proto.h"
#include "../stream.h"

#include <netinet/in.h>

void removeHeartbeatFromList(lHeartbeat**, lHeartbeat*);
void freeHeartbeatList(lHeartbeat*);
void handleSentHeartbeat(lHeartbeat**, lHeartbeat*);
void handleReceivedHeartbeat(heartbeat*, struct in_addr, tStream*);
int checkMatchedHeartbeat(lHeartbeat**, response*);
void removeTimedoutHeartbeats(lHeartbeat**);

#endif

