#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <netinet/in.h>
#include <stdint.h>

#define NETWORKFLAGS_BCAST 1

int setupSocket(int);
int createAddr(uint32_t, struct sockaddr_in*);

#endif
