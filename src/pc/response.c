#include "pc.h"
#include "response.h"
#include "heartbeat.h"
#include "../proto.h"

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

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

	if(checkMatchedHeartbeat(sent, r) == 0 && handleUnmatchedResponse(unmatched, r) == NULL) {
		return -1;
	}

	return 0;
}
