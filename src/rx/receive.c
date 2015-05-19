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

#include "receive.h"

#include <dhcpext/pc.h>

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
		return -1;
	}

	if(size == 0) {
		return 0;
			// TODO: Handle this properly. As of right now, an
			// empty broadcast packet will crash the entire network,
			// necessitating a full system restart. This needs
			// fixing.
	}

	char* buffer = malloc(size);

	if(buffer == NULL) {
		return -1;
	}

	ssize_t rc;

	socklen_t addrlen = sizeof(replyaddr);
	rc = recvfrom(sd, (char*)buffer, size, MSG_DONTWAIT,
		(struct sockaddr*)&replyaddr, &addrlen);

	if(rc < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		return -1;
	} else if(rc >= 0) {
		message* m = malloc(sizeof(message));

		if(m == NULL) {
			return -1;
		}

		m->buffer = buffer;
		m->addrv4 = replyaddr.sin_addr.s_addr;
		m->bufferSize = size;

		if(send(pc_sd, (char*)m, sizeof(message), 0) == -1) {
			return -1;
		}

		free(m);
	}

	return 0;
}

