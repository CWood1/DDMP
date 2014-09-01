#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <netinet/in.h>

#define NETWORKFLAGS_BCAST 1

int setupSocket(int);
int createAddr(char*, struct sockaddr_in*);

#endif
