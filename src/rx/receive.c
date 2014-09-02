#include "receive.h"

#include <dhcpext/pc.h>
#include <dhcpext/stream.h>

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

int receive(struct sockaddr_in replyaddr, int sd, tStream* pcStream) {
	char buffer[100];
	int rc;

	int addrlen = sizeof(replyaddr);
	rc = recvfrom(sd, (char*)buffer, sizeof(buffer), MSG_DONTWAIT,
		(struct sockaddr*)&replyaddr, &addrlen);

	if(rc < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		return 1;
	} else if(rc >= 0) {
		message* m = malloc(sizeof(message));

		if(m == NULL) {
			return 1;
		}

		m->buffer = malloc(rc);

		if(m->buffer == NULL) {
			return 1;
		}

		memcpy(m->buffer, (void*)buffer, rc);
		m->addrv4 = replyaddr.sin_addr.s_addr;
		m->bufferSize = rc;

		stream_send(pcStream, (char*)m, sizeof(message));
		free(m);
	}

	return 0;
}

