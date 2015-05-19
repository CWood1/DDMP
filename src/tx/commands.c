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

#include "commands.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>

int handleCommands(int sd) {
	size_t size;
	ssize_t rc;
	char* cmd;

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return -1;
	}

	if(size == 0) {
		errno = ENOMSG;
		return -1;
	}

	cmd = malloc(size);

	if(cmd == NULL) {
		return -1;
	}

	rc = recv(sd, cmd, size, 0);

	if(rc < 0) {
		return -1;
	}

	char* s = strtok(cmd, " ");

	if(strcmp(s, "shutdown") == 0) {
		free(cmd);
		return 1;
	}

	if(strcmp(s, "setbcast") == 0) {
		s = strtok(NULL, " ");

		if(strcmp(s, "1") == 0) {
			free(cmd);
			return 2;
		}

		if(strcmp(s, "0") == 0) {
			free(cmd);
			return 3;
		}
	}

	return 0;
}
