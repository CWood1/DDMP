#ifndef __COMMON_H__
#define __COMMON_H__

#include <dhcpext/stream.h>

#include <netinet/in.h>
#include <stdint.h>

#define PORT 4123

#define NETWORKFLAGS_BCAST 1

tStream* getStreamFromStream(tStream*);
tStream* getStreamFromSock(int);
int setupSocket(int);
int createAddr(uint32_t, struct sockaddr_in*);

#endif
