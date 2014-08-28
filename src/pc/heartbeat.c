#include "heartbeat.h"
#include "../proto.h"
#include "../stream.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

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

void handleReceivedHeartbeat(heartbeat* h, struct in_addr addrv4, tStream* rpStream) {
	printf("Heartbeat received(%s):\n", inet_ntoa(addrv4));
	printHeartbeat(h);
	stream_send(rpStream, (char*)h, sizeof(heartbeat));
}

int checkMatchedHeartbeat(lHeartbeat** sent, response* r) {
	lHeartbeat* cur = *sent;

	if(cur == NULL) {
		return 0;
	}

	while(cur->h->magic != r->magic) {
		if(cur->next != NULL) {
			cur = cur->next;
		} else {
			return 0;
		}
	}

	printf("Response matches:\n");
	printHeartbeat(cur->h);

	if(cur->next != NULL) {
		cur->next->prev = cur->prev;
	}

	if(cur->prev != NULL) {
		cur->prev->next = cur->next;
	}

	if(cur == *sent) {
		*sent = cur->next;
	}

	free(cur->h);
	free(cur);
	free(r);

	return 1;
}
