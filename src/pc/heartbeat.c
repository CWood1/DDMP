#include "heartbeat.h"
#include "../proto.h"
#include "../stream.h"

#include <netinet/in.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

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

	free(cur->h);
	free(cur);
}

void freeHeartbeatList(lHeartbeat* list) {
	lHeartbeat* cur = list;

	while(cur != NULL) {
		lHeartbeat* next = cur->next;

		free(cur->h);
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
			return 1;
		}
	}

	printf("Response matches:\n");
	printHeartbeat(cur->h);

	removeHeartbeatFromList(sent, cur);
	free(r);

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
