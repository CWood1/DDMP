#ifndef __PROTO_H__
#define __PROTO_H__

typedef struct {
	char* s;
} heartbeat;

typedef struct {
	char* s;
} response;

heartbeat* craftHeartbeat(int);
char* serializeHeartbeat(heartbeat*, int*);
heartbeat* deserializeHeartbeat(char*);
void printHeartbeat(heartbeat*);

response* craftResponse(heartbeat*);
char* serializeResponse(response*, int*);
response* deserializeResponse(char*);
void printResponse(response*);

int isHeartbeatOrResponse(char*);

#endif
