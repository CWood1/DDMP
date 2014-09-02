#include "api.h"
#include "heartbeat.h"
#include "response.h"

#include <dhcpext/pc.h>
#include <dhcpext/stream.h>

#include <stdlib.h>
#include <netinet/in.h>

void getSentHeartbeats(lHeartbeat** sent, tStream* txStream) {
	int len;
	lHeartbeat* next = (lHeartbeat*)(stream_rcv_nblock(txStream, &len));

	while(next != NULL) {
		handleSentHeartbeat(sent, next);
		next = (lHeartbeat*)(stream_rcv_nblock(txStream, &len));
	}
}

int getReceivedMessages(tStream* rxStream, tStream* rpStream,
		lHeartbeat** sent, lResponse** unmatched) {
	int len;
	message* m = (message*)(stream_rcv_nblock(rxStream, &len));

	while(m != NULL) {
		struct in_addr addrv4;
		addrv4.s_addr = m->addrv4;

		if(isHeartbeat(m->buffer, m->bufferSize)) {
			heartbeat* h = deserializeHeartbeat(m->buffer, m->bufferSize);
			handleReceivedHeartbeat(h, addrv4, rpStream);
			freeHeartbeat(h);
		} else {
			response* r = deserializeResponse(m->buffer, m->bufferSize);
			if(handleResponse(r, sent, unmatched, addrv4) == -1) {
				return -1;
			}
		}

		free(m->buffer);
		free(m);

		m = (message*)(stream_rcv_nblock(rxStream, &len));
	}

	return 0;
}
	
