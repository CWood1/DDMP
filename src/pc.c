#include "pc.h"
#include "stream.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

void* pcmain(void* s) {
	tStream* cmdStream = (tStream*) s;

	tStream** pTxStream = malloc(stream_wait_full(cmdStream));
		// First piece of data to come down is a pointer to the stream
		// from tx
	stream_rcv(cmdStream, 0, (char*)pTxStream);

	tStream* txStream = *pTxStream;
	free(pTxStream);

	tStream** pRxStream = malloc(stream_wait_full(cmdStream));
	stream_rcv(cmdStream, 0, (char*)pRxStream);

	tStream* rxStream = *pRxStream;
	free(pRxStream);

	int sd;
	struct sockaddr_in replyaddr;

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Processing - socket error.");
		pthread_exit(NULL);
	}

	memset(&replyaddr, 0, sizeof(replyaddr));
	replyaddr.sin_family = AF_INET;
	replyaddr.sin_port = htons(PORT);

	while(1) {
		lHeartbeat* sent = malloc(sizeof(lHeartbeat));
		sent->next = NULL;
		sent->prev = NULL;
		sent->h = NULL;

		lHeartbeat* received = malloc(sizeof(lHeartbeat));
		sent->next = NULL;
		sent->prev = NULL;
		sent->h = NULL;

		int size = stream_length(cmdStream);
		if(size != 0) {
			char* cmd = malloc(stream_wait_full(cmdStream));
			stream_rcv(cmdStream, 0, cmd);

			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
				printf("pc shutting down.\n");
				close(sd);
				pthread_exit(NULL);
			}
		}

		size = stream_length(txStream);
		if(size != 0) {
			heartbeat** h = malloc(stream_wait_full(txStream));
			stream_rcv(txStream, 0, (char*) h);

			sent->h = *h;
			sent->next = malloc(sizeof(lHeartbeat));
			sent->next->prev = sent;
			sent = sent->next;

			free(h);
		}

		size = stream_length(rxStream);
		if(size != 0) {
			char* in = malloc(stream_wait_full(rxStream));
			stream_rcv(rxStream, 0, in);

			message* m = (message*)in;
			replyaddr.sin_addr.s_addr = m->addrv4;
				// Needed to display IP addresses properly

			if(isHeartbeat(m->buffer, m->bufferSize)) {
				printf("Heartbeat received (%s):\n",
					inet_ntoa(replyaddr.sin_addr));

				heartbeat* h = deserializeHeartbeat(m->buffer, m->bufferSize);
				printHeartbeat(h);

				int size;

				response* r = craftResponse(h);
				char* b = serializeResponse(r, &size);

				int rc = sendto(sd, b, size, 0,
					(struct sockaddr*)&replyaddr, sizeof(replyaddr));

				free(r);
				free(b);

				if(rc < 0) {
					perror("sendto error");
					close(sd);
					pthread_exit(NULL);
				}
			} else {
				printf("Response received (%s):\n",
					inet_ntoa(replyaddr.sin_addr));

				response* r = deserializeResponse(m->buffer, m->bufferSize);
				printResponse(r);
				free(r);
	 		}
		}
	}
}
