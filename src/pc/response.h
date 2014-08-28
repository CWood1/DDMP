#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "pc.h"
#include "../proto.h"

#include <netinet/in.h>

void* handleUnmatchedResponse(lResponse**, response*);
int handleResponse(response*, lHeartbeat**, lResponse**, struct in_addr);

#endif
