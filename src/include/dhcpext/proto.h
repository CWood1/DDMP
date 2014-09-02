#ifndef __PROTO_H__
#define __PROTO_H__

#include <stdint.h>

typedef struct {
	uint32_t filelen;
	char* file;
} leasefile;

typedef struct nodelist {
	char* ip;
	struct nodelist* next;
} nodelist;

typedef struct {
	uint8_t ident;
	uint32_t flags;
	uint32_t magic;

	leasefile* l;
	nodelist* n;
} heartbeat;

typedef struct {
	uint8_t ident;
	uint32_t flags;
	uint32_t magic;

	uint32_t numnodes;
	uint8_t shutdown;
		// Only ever used if D flag is set
} response;

#define FLAG_ACTIVE 1
#define FLAG_SHUTDOWN 2

heartbeat* craftHeartbeat(int);
char* serializeHeartbeat(heartbeat*, int*);
heartbeat* deserializeHeartbeat(char*, int);
void printHeartbeat(heartbeat*);
void freeHeartbeat(heartbeat*);

response* craftResponse(heartbeat*);
char* serializeResponse(response*, int*);
response* deserializeResponse(char*, int);
void printResponse(response*);
void freeHeartbeat(heartbeat*);

int isHeartbeat(char*, int);

#endif
