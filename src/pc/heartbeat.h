#ifndef __HEARTBEAT_H__
#define __HEARTBEAT_H__

#include "../proto.h"
#include "../stream.h"
#include "pc.h"

#include <netinet/in.h>

void handleSentHeartbeat(lHeartbeat**, lHeartbeat*);
void handleReceivedHeartbeat(heartbeat*, struct in_addr, tStream*);
int checkMatchedHeartbeat(lHeartbeat**, response*);

#endif

