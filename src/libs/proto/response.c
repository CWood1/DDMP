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

#include <dhcpext/proto.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

response* craftResponse(heartbeat* h) {
	response* r = malloc(sizeof(response));

	if(r == NULL) {
		return NULL;
	}

	r->ident = 1;
	r->flags = 0;
	r->magic = h->magic;

	return r;
}

char* serializeResponse(response* r, unsigned int* length) {
	char* s = malloc(9);

	if(s == NULL) {
		return NULL;
	}

	memset(s, 0, 9);

	memcpy(s, &(r->ident), sizeof(r->ident));
	memcpy(s + 1, &(r->flags), sizeof(r->flags));
	memcpy(s + 5, &(r->magic), sizeof(r->magic));

	*length = 9;
	return s;
}

response* deserializeResponse(char* s, size_t length) {
	if(length != 9) {
		errno = EINVAL;
		return NULL;
	}

	response* r = malloc(sizeof(response));

	if(r == NULL) {
		return NULL;
	}

	memcpy(&(r->ident), s, sizeof(r->ident));
	memcpy(&(r->flags), s + 1, sizeof(r->flags));
	memcpy(&(r->magic), s + 5, sizeof(r->magic));

	return r;
}

void printResponse(response* r) {
	printf("\tType:\t%s\n\tFlags:\t%x\n\tMagic:\t%x\n",
		((r->ident == 1) ? "Response" : ""),
		r->flags, r->magic);
}

void freeResponse(response* r) {
	if(r == NULL) {
		return;
	}

	free(r);
}
