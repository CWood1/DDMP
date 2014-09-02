#ifndef __PC_H__
#define __PC_H__

#include <dhcpext/proto.h>

#include <stdint.h>
#include <stddef.h>
#include <sys/time.h>

typedef struct lHeartbeat {
	struct lHeartbeat* next;
	struct lHeartbeat* prev;

	heartbeat* h;
	uint32_t addrv4;
	struct timeval timeSent;
} lHeartbeat;

typedef struct message {
	char* buffer;
	size_t bufferSize;
	uint32_t addrv4;
} message;

void* pcmain(void*);

#endif
