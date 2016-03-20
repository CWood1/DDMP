/**
    This file is part of mesh-dhcp-ext, the Mesh Network DHCP Extensions protocol.

    mesh-dhcp-ext is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    mesh-dhcp-ext is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with mesh-dhcp-ext.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2015 Connor Wood <connorwood71@gmail.com>.
*/

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
	heartbeat* h;
	lHeartbeat* s;
	char* buffer;

	if(flags & TXFLAGS_BCAST) {
		iFlags |= FLAG_ACTIVE;
	}

	if((h = craftHeartbeat(iFlags)) == NULL) {
		return NULL;
	}

	if((buffer = serializeHeartbeat(h, &length)) == NULL) {
		return NULL;
	}

	if(sendto(sd, buffer, length, 0,
			(struct sockaddr*)&addr, sizeof(addr)) < 0) {
		return NULL;
	}

	free(buffer);

	if((s = malloc(sizeof(lHeartbeat))) == NULL) {
		return NULL;
	}

	s->next = NULL;
	s->prev = NULL;
	s->h = h;
	s->addrv4  = addr.sin_addr.s_addr;

	if(gettimeofday(&(s->timeSent), NULL) == -1) {
		return NULL;
	}

	if(send(pc_sd, (char*)s, sizeof(lHeartbeat), 0) == -1) {
		return NULL;
	}

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
