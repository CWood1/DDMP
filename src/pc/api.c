#include "api.h"
#include "heartbeat.h"
#include "response.h"

#include <dhcpext/pc.h>

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

int getSentHeartbeats(lHeartbeat** sent, int tx_sd) {
	size_t size;
	lHeartbeat* next;

	if(ioctl(tx_sd, FIONREAD, &size) != 0) {
		return -1;
	}

	if(size == 0) {
		return -1;
	}

	next = malloc(size);

	if(next == NULL) {
		return -1;
	}

	if(recv(tx_sd, (char*)next, size, 0) < 0) {
		return -1;
	}

	handleSentHeartbeat(sent, next);
	return 0;
}

int getReceivedMessages(int rx_sd, int rp_sd,
		lHeartbeat** sent, lResponse** unmatched) {
	size_t size;
	message* m;
	struct in_addr addrv4;

	if(ioctl(rx_sd, FIONREAD, &size) != 0) {
		return -1;
	}

	if(size == 0) {
		return -1;
	}

	m = malloc(size);

	if(m == NULL) {
		return -1;
	}

	if(recv(rx_sd, (char*)m, size, 0) < 0) {
		return -1;
	}

	addrv4.s_addr = m->addrv4;

	if(isHeartbeat(m->buffer, m->bufferSize)) {
		heartbeat* h = deserializeHeartbeat(m->buffer, m->bufferSize);
		handleReceivedHeartbeat(h, addrv4, rp_sd);
		freeHeartbeat(h);
		// TODO: if isHeartbeat returns -1, packet is invalid. Handle.
	} else {
		response* r = deserializeResponse(m->buffer, m->bufferSize);
		if(handleResponse(r, sent, unmatched, addrv4) == -1) {
			return -1;
		}
	}

	free(m->buffer);
	free(m);

	return 0;
}
	
