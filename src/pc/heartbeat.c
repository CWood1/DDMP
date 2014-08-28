#include "pc.h"
#include "heartbeat.h"
#include "../proto.h"
#include "../stream.h"

#include <netinet/in.h>
#include <stdio.h>

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
