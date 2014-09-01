#include "transmit.h"

#include <dhcpext/tx.h>
#include <dhcpext/pc.h>
#include "../stream.h"
#include "../proto.h"

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

lHeartbeat* sendHeartbeat(int sd, struct sockaddr_in addr, tStream* pcStream, int flags) {
	int iFlags = 0;
	int length;

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

	stream_send(pcStream, (char*)s, sizeof(lHeartbeat));
	return s;
}

int sendHeartbeats(int flags, int sd, struct sockaddr_in bcastaddr,
		struct sockaddr_in directaddr, tStream* pcStream) {
	static struct timeval last = {0, 0};

	struct timeval now;
	gettimeofday(&now, NULL);

	if(last.tv_sec == 0 ||
			last.tv_sec < now.tv_sec || (now.tv_usec - last.tv_usec) >= 100000) {
		lHeartbeat* sent;

		if(flags & TXFLAGS_BCAST) {
			sent = sendHeartbeat(sd, bcastaddr, pcStream, flags);

			if(sent == NULL) {
				return 1;
			}

			free(sent);
		}

		sent = sendHeartbeat(sd, directaddr, pcStream, flags & ~TXFLAGS_BCAST);

		if(sent == NULL) {
			return 2;
		}

		last = sent->timeSent;
		free(sent);
	}

	return 0;
}
