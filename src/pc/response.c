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

#include "response.h"
#include "heartbeat.h"

#include <dhcpext/pc.h>
#include <dhcpext/proto.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void removeResponseFromList(lResponse** list, lResponse* cur) {
	if(cur->next != NULL) {
		cur->next->prev = cur->prev;
	}

	if(cur->prev != NULL) {
		cur->prev->next = cur->next;
	}

	if(cur == *list) {
		*list = cur->next;
	}

	free(cur->r);
	free(cur);
}

void freeResponseList(lResponse* list) {
	lResponse* cur = list;

	while(cur != NULL) {
		lResponse* next = cur->next;

		free(cur->r);
		free(cur);

		cur = next;
	}
}

void* handleUnmatchedResponse(lResponse** unmatched, response* r) {
	lResponse* cur = *unmatched;

	if(cur == NULL) {
		*unmatched = malloc(sizeof(lResponse));

		if(*unmatched == NULL) {
			return NULL;
		}

		cur = *unmatched;
		cur->prev = NULL;
	} else {
		while(cur->next != NULL) {
			cur = cur->next;
		}

		cur->next = malloc(sizeof(lResponse));

		if(cur->next == NULL) {
			return NULL;
		}

		cur->next->prev = cur;
		cur = cur->next;
	}

	cur->next = NULL;
	cur->r = r;
	cur->counter = 0;
	return cur;
}

int handleResponse(response* r, lHeartbeat** sent, lResponse** unmatched, struct in_addr addrv4) {
	printf("Response received (%s):\n", inet_ntoa(addrv4));
	printResponse(r);

	if(checkMatchedHeartbeat(sent, r) == 1) {
		if(handleUnmatchedResponse(unmatched, r) == NULL) {
			return -1;
		}
	} else {
		free(r);
	}

	return 0;
}

void checkUnmatchedList(lResponse** unmatched, lHeartbeat** sent) {
	lResponse* cur = *unmatched;

	while(cur != NULL) {
		if(cur->counter++ == 0) {
			cur = cur->next;
		} else {
			if(checkMatchedHeartbeat(sent, cur->r) == 0) {
				printf("Response unmatched:\n");
				printResponse(cur->r);
			} else {
				printf("Response now matched:\n");
				printResponse(cur->r);
			}

			lResponse* next = cur->next;
			removeResponseFromList(unmatched, cur);
			cur = next;
		}
	}
}
