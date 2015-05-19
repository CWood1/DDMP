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

#include "heartbeat.h"

#include <dhcpext/proto.h>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>

heartbeat* getHeartbeatFromSock(int sd) {
	size_t size;
	heartbeat* h;

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return NULL;
	}

	if(size == 0) {
		errno = ENOMSG;
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
