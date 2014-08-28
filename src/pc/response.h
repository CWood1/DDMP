#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "heartbeat.h"
#include "../proto.h"

#include <stdint.h>
#include <netinet/in.h>

typedef struct lResponse {
	struct lResponse* next;
	struct lResponse* prev;

	response* r;
	uint8_t counter;
} lResponse;

void* handleUnmatchedResponse(lResponse**, response*);
int handleResponse(response*, lHeartbeat**, lResponse**, struct in_addr);

#endif
