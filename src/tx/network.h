#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <netinet/in.h>

int setupSocket();
int createAddr(char*, struct sockaddr_in*);

#endif
