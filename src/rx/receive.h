#ifndef __RECEIVE_H__
#define __RECEIVE_H__

#include "../stream.h"

#include <netinet/in.h>

int receive(struct sockaddr_in, int, tStream*);

#endif