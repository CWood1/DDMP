#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#include "../pc/pc.h"
#include "../stream.h"

#include <netinet/in.h>

lHeartbeat* sendHeartbeat(int, struct sockaddr_in, tStream*, int);
int sendHeartbeats(int, int, struct sockaddr_in, struct sockaddr_in, tStream*);

#endif
