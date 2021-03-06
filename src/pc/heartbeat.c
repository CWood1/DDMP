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

#include <dhcpext/pc.h>
#include <dhcpext/proto.h>

#include <netinet/in.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void removeHeartbeatFromList(lHeartbeat** list, lHeartbeat* cur) {
	if(cur->next != NULL) {
		cur->next->prev = cur->prev;
	}

	if(cur->prev != NULL) {
		cur->prev->next = cur->next;
	}

	if(cur == *list) {
		*list = cur->next;
	}

	freeHeartbeat(cur->h);
	free(cur);
}

void freeHeartbeatList(lHeartbeat* list) {
	lHeartbeat* cur = list;

	while(cur != NULL) {
		lHeartbeat* next = cur->next;

		freeHeartbeat(cur->h);
		free(cur);

		cur = next;
	}
}

void handleSentHeartbeat(lHeartbeat** sent, lHeartbeat* next) {
	struct in_addr addrv4;
	addrv4.s_addr = next->addrv4;

	next->next = NULL;

	if(*sent == NULL) {
		*sent = next;
		next->prev = NULL;
	} else {
		lHeartbeat* cur = *sent;

		while(cur->next != NULL) {
			cur = cur->next;
		}

		cur->next = next;
		next->prev = cur;
	}

	printf("Heartbeat sent(%s):\n", inet_ntoa(addrv4));
	printHeartbeat(next->h);
}

void handleReceivedHeartbeat(heartbeat* h, struct in_addr addrv4, int rp_sd) {
	printf("Heartbeat received(%s):\n", inet_ntoa(addrv4));
	printHeartbeat(h);
	send(rp_sd, h, sizeof(heartbeat), 0);
}

int checkMatchedHeartbeat(lHeartbeat** sent, response* r) {
	lHeartbeat* cur = *sent;

	if(cur == NULL) {
		return 1;
	}

	if(r == NULL) {
		return 1;
	}

	while(cur->h->magic != r->magic) {
		if(cur->next != NULL) {
			cur = cur->next;
		} else {
			return 1;
		}
	}

	printf("Response matches:\n");
	printHeartbeat(cur->h);

	removeHeartbeatFromList(sent, cur);

	return 0;
}

void removeTimedoutHeartbeats(lHeartbeat** sent) {
	lHeartbeat* cur = *sent;

	if(cur == NULL) {
		return;
	}

	struct timeval now;
	gettimeofday(&now, NULL);

	while(cur != NULL) {
		if(cur->timeSent.tv_sec > now.tv_sec ||
				now.tv_usec - cur->timeSent.tv_usec > 400000) {
				// If more than 400ms has passed, remove from list
			printf("Timedout heartbeat:\n");
			printHeartbeat(cur->h);

			lHeartbeat* next = cur->next;
			removeHeartbeatFromList(sent, cur);
			cur = next;
		} else {
			cur = cur->next;
		}
	}
}
