#ifndef __API_H__
#define __API_H__

#include "heartbeat.h"
#include "response.h"
#include "../stream.h"

typedef struct message {
	char* buffer;
	uint32_t bufferSize;
	uint32_t addrv4;
		// IPv6 is not supported, at this stage
} message;

void getSentHeartbeats(lHeartbeat**, tStream*);
int getReceivedMessages(tStream*, tStream*, lHeartbeat**, lResponse**);

#endif
