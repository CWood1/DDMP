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

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>

int getConfig(int sd, char** str_bcastaddr, char** str_directaddr,
		int* flags) {
	size_t size;
	fd_set set;

	FD_ZERO(&set);
	FD_SET(sd, &set);

	if(select(sd + 1, &set, NULL, NULL, NULL) == -1) {
		return -1;
	}

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return -1;
	}

	char* str_config = malloc(size);

	if(str_config == NULL) {
		return -1;
	}

	if(recv(sd, str_config, size, 0) < 0) {
		return -1;
	}

	FD_ZERO(&set);
	FD_SET(sd, &set);

	if(select(sd + 1, &set, NULL, NULL, NULL) == -1) {
		return -1;
	}

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return -1;
	}

	int* pFlags = malloc(size);

	if(pFlags == NULL) {
		return -1;
	}

	if(recv(sd, (char*)pFlags, size, 0) < 0) { 
		return -1;
	}

	char* tstr_bcastaddr = strtok(str_config, " ");
	char* tstr_directaddr = strtok(NULL, " ");

	if(tstr_bcastaddr == NULL) {
		errno = ENOMSG;
		return -1;
	}

	if(tstr_directaddr == NULL) {
		errno = ENOMSG;
		return -1;
	}

	*str_bcastaddr = malloc(strlen(tstr_bcastaddr) + 1);

	if(*str_bcastaddr == NULL) {
		return -1;
	}

	strcpy(*str_bcastaddr, tstr_bcastaddr);
	*str_directaddr = malloc(strlen(tstr_directaddr) + 1);

	if(*str_directaddr == NULL) {
		return -1;
	}

	strcpy(*str_directaddr, tstr_directaddr);
	free(str_config);

	*flags = *pFlags;
	free(pFlags);

	return 0;
}
