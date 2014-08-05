#ifndef __PROTO_H__
#define __PROTO_H__

#include <stdint.h>

typedef struct {
	int filelen;
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
	char* s;
} response;

heartbeat* craftHeartbeat(int);
char* serializeHeartbeat(heartbeat*, int*);
heartbeat* deserializeHeartbeat(char*, int);
void printHeartbeat(heartbeat*);

response* craftResponse(heartbeat*);
char* serializeResponse(response*, int*);
response* deserializeResponse(char*);
void printResponse(response*);

int isHeartbeat(char*, int);

#endif
