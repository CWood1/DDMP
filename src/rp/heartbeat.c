#include "heartbeat.h"

#include <dhcpext/proto.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

heartbeat* getHeartbeatFromSock(int sd) {
	size_t size;
	heartbeat* h;

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return NULL;
	}

	if(size == 0) {
		return NULL;
	}

	h = malloc(size);

	if(h == NULL) {
		return NULL;
	}

	if(recv(sd, h, size, 0) < 0) {
		return NULL;
	}

	return h;
}
