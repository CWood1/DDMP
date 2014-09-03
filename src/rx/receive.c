#include "receive.h"

#include <dhcpext/pc.h>
#include <dhcpext/stream.h>

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <netinet/udp.h>

int receive(struct sockaddr_in replyaddr, int sd, int pc_sd) {
	size_t size;

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return 1;
	}

	if(size == 0) {
		return 0;
	}

	char* buffer = malloc(size);

	if(buffer == NULL) {
		return 1;
	}

	ssize_t rc;

	socklen_t addrlen = sizeof(replyaddr);
	rc = recvfrom(sd, (char*)buffer, size, MSG_DONTWAIT,
		(struct sockaddr*)&replyaddr, &addrlen);

	if(rc < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		return 1;
	} else if(rc >= 0) {
		message* m = malloc(sizeof(message));

		if(m == NULL) {
			return 1;
		}

		m->buffer = buffer;
		m->addrv4 = replyaddr.sin_addr.s_addr;
		m->bufferSize = size;

		send(pc_sd, (char*)m, sizeof(message), 0);
		free(m);
	}

	return 0;
}

