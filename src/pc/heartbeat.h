#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include "../proto.h"
#include "../stream.h"

#include <stdint.h>
#include <sys/time.h>
#include <netinet/in.h>

typedef struct lHeartbeat {
	struct lHeartbeat* next;
	struct lHeartbeat* prev;

	heartbeat* h;
	uint32_t addrv4;
	struct timeval timeSent;
} lHeartbeat;

void removeHeartbeatFromList(lHeartbeat**, lHeartbeat*);
void freeHeartbeatList(lHeartbeat*);
void handleSentHeartbeat(lHeartbeat**, lHeartbeat*);
void handleReceivedHeartbeat(heartbeat*, struct in_addr, tStream*);
int checkMatchedHeartbeat(lHeartbeat**, response*);
void removeTimedoutHeartbeats(lHeartbeat**);

#endif

