#ifndef __PC_H__
#define __PC_H__

#include "proto.h"
#include <stdint.h>

typedef struct lHeartbeat {
	struct lHeartbeat* next;
	struct lHeartbeat* prev;

	heartbeat* h;
	uint32_t addrv4;
} lHeartbeat;

typedef struct message {
	char* buffer;
	uint32_t bufferSize;
	uint32_t addrv4;
		// IPv6 is currently unsupported
} message;
	// This structure may be later moved to common.h

void* pcmain(void*);

#endif
