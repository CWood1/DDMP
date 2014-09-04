#include "transmit.h"

#include <dhcpext/tx.h>
#include <dhcpext/pc.h>
#include <dhcpext/proto.h>

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

lHeartbeat* sendHeartbeat(int sd, struct sockaddr_in addr, int pc_sd, int flags) {
	int iFlags = 0;
	unsigned int length;

	if(flags & TXFLAGS_BCAST) {
		iFlags |= FLAG_ACTIVE;
	}

	heartbeat* h = craftHeartbeat(iFlags);
	char* buffer = serializeHeartbeat(h, &length);

	int rc = sendto(sd, buffer, length, 0,
		(struct sockaddr*)&addr, sizeof(addr));
	free(buffer);

	if(rc < 0) {
		return NULL;
	}

	lHeartbeat* s = malloc(sizeof(lHeartbeat));

	if(s == NULL) {
		return NULL;
	}

	s->next = NULL;
	s->prev = NULL;
	s->h = h;
	s->addrv4  = addr.sin_addr.s_addr;
	gettimeofday(&(s->timeSent), NULL);

	send(pc_sd, (char*)s, sizeof(lHeartbeat), 0);
	return s;
}

int sendHeartbeats(int flags, int sd, struct sockaddr_in bcastaddr,
		struct sockaddr_in directaddr, int pc_sd) {
	lHeartbeat* sent;

	if(flags & TXFLAGS_BCAST) {
		sent = sendHeartbeat(sd, bcastaddr, pc_sd, flags);

		if(sent == NULL) {
			return 1;
		}

		free(sent);
	}

	sent = sendHeartbeat(sd, directaddr, pc_sd, flags & ~TXFLAGS_BCAST);

	if(sent == NULL) {
		return 2;
	}

	free(sent);

	return 0;
}
