#ifndef __TRANSMIT_H__
#define __TRANSMIT_H__

#include <dhcpext/pc.h>
#include <dhcpext/stream.h>

#include <netinet/in.h>

lHeartbeat* sendHeartbeat(int, struct sockaddr_in, int, int);
int sendHeartbeats(int, int, struct sockaddr_in, struct sockaddr_in, int);

#endif
