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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

heartbeat* craftHeartbeat(int active) {
	heartbeat* h = malloc(sizeof(heartbeat));

	if(h == NULL) {
		return NULL;
	}

	h->ident = 0;
	h->flags = 0;

	if(active == 1) {
		h->flags |= FLAG_ACTIVE;
	}
	
	int random = open("/dev/urandom", O_RDONLY);

	if(random == -1) {
		return NULL;
	}
	
	if(read(random, &(h->magic), sizeof(h->magic)) == -1) {
		return NULL;
	}

	if(close(random) == -1) {
		return NULL;
	}

	h->l = NULL;
	h->n = NULL;

	return h;
}

char* serializeHeartbeat(heartbeat* h, unsigned int* length) {
	char* s = malloc(9);

	if(s == NULL) {
		return NULL;
	}

	memset(s, 0, 9);
	memcpy(s, &(h->ident), sizeof(h->ident));
	memcpy(s + 1, &(h->flags), sizeof(h->flags));
	memcpy(s + 5, &(h->magic), sizeof(h->magic));

	*length = 9;
	return s;
}

heartbeat* deserializeHeartbeat(char* s, size_t length) {
	if(length != 9) {
		return NULL;
	}

	heartbeat* h = malloc(sizeof(heartbeat));

	if(h == NULL) {
		return NULL;
	}

	memset(h, 0, sizeof(heartbeat));
	memcpy(&(h->ident), s, sizeof(h->ident));
	memcpy(&(h->flags), s + 1, sizeof(h->flags));
	memcpy(&(h->magic), s + 5, sizeof(h->magic));

	h->l = NULL;
	h->n = NULL;

	return h;
}

void printHeartbeat(heartbeat* h) {
	printf("\tType:\t%s\n\tFlags:\t%x\n\tMagic:\t%x\n", 
		((h->ident == 0) ? "Heartbeat" : ""),
		h->flags, h->magic);
}

void freeHeartbeat(heartbeat* h) {
	if(h == NULL) {
		return;
	}
		// Doesn't warrant an error, but don't try to access
		// unallocated memory

	if(h->l != NULL) {
		free(h->l);
	}

	if(h->n != NULL) {
		free(h->n);
	}
		// Both of these are crude, and do not free a linked list.
		// When UCI integration happens, these will need rewriting.

	free(h);
}
