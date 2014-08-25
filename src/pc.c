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
	int len;
	char running = 1;

	tStream* cmdStream = (tStream*) s;
	tStream** pTxStream = (tStream**)(stream_rcv(cmdStream, &len));

	if(len != sizeof(tStream*)) {
		printf("Error receiving TX/PC stream\n");
		pthread_exit(NULL);
	}

	tStream* txStream = *pTxStream;
	free(pTxStream);

	tStream** pRxStream = (tStream**)(stream_rcv(cmdStream, &len));

	if(len != sizeof(tStream*)) {
		printf("Error receiving RX/PC stream\n");
		pthread_exit(NULL);
	}

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

	lHeartbeat* sent = NULL;

	while(running) {
		char* cmd = stream_rcv_nblock(cmdStream, &len);

		if(cmd != NULL) {
			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
				running = 0;
			}

			free(cmd);
		}

		lHeartbeat* next = (lHeartbeat*)(stream_rcv_nblock(txStream, &len));

		while(next != NULL) {
			lHeartbeat* cur;

			if(sent == NULL) {
				sent = next;

				sent->prev = NULL;
				sent->next = NULL;

				cur = sent;
			} else {
				lHeartbeat* cur = sent;

				while(cur->next != NULL) {
					cur = cur->next;
				}

				cur->next = next;
				next->prev = cur;
				next->next = NULL;

				cur = cur->next;
			}

			replyaddr.sin_addr.s_addr = next->addrv4;
				// A kludge, but we need to display this

			printf("Heartbeat sent (%s):\n",
				inet_ntoa(replyaddr.sin_addr));
			printHeartbeat(next->h);

			next = (lHeartbeat*)(stream_rcv_nblock(txStream, &len));
		}

		message* m = (message*)(stream_rcv_nblock(rxStream, &len));

		while(m != NULL) {
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

				free(h);
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

				lHeartbeat* cur = sent;

				while(cur->h->magic != r->magic) {
					if(cur->next != NULL) {
						cur = cur->next;
					}
				}

				if(cur->h->magic == r->magic) {
					if(cur->next != NULL) {
						cur->next->prev = cur->prev;
					}

					if(cur->prev != NULL) {
						cur->prev->next = cur->next;
					}

					if(cur == sent) {
						sent = cur->next;
					}

					free(cur->h);
					free(cur);
				}

				free(r);
	 		}

			free(m->buffer);
			free(m);
			m = (message*)(stream_rcv_nblock(rxStream, &len));
		}
	}

	lHeartbeat* cur = sent;
	while(cur->next != NULL) {
		free(cur->h);
		cur = cur->next;
		free(cur->prev);
	}

	free(cur->h);
	free(cur);

	printf("pc shutting down.\n");
	close(sd);
	pthread_exit(NULL);
}
