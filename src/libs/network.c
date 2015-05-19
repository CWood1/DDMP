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

#include <dhcpext/common.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int setupSocket(int flags) {
	int sd;
	int broadcast = 1;

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	if(flags & NETWORKFLAGS_BCAST) {
		if(setsockopt(sd, SOL_SOCKET, SO_BROADCAST,
				(void*)&broadcast, sizeof(broadcast)) < 0) {
			return -1;
		}
	}

	return sd;
}

int createAddr(uint32_t addr, struct sockaddr_in* saddr) {
	if(saddr == NULL) {
		errno = EINVAL;
		return -1;
	}

	memset(saddr, 0, sizeof(struct sockaddr_in));
	saddr->sin_family = AF_INET;
	saddr->sin_port = htons(PORT);

	if((saddr->sin_addr.s_addr = addr) == (unsigned long)INADDR_NONE) {
		return -1;
	}

	return 0;
}
