#include "pc.h"
#include "heartbeat.h"
#include "response.h"
#include "../stream.h"
#include "../common.h"

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
			free(h);
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

void* pcmain(void* s) {
	int len;
	char running = 1;

	tStream* cmdStream = (tStream*) s;
	tStream* txStream = getStreamFromStream(cmdStream);
	tStream* rxStream = getStreamFromStream(cmdStream);
	tStream* rpStream = getStreamFromStream(cmdStream);

	if(txStream == NULL) {
		printf("PC:\tUnable to receive stream to TX\n");
		pthread_exit(NULL);
	}

	if(rxStream == NULL) {
		printf("PC:\tUnable to receive stream to RX\n");
		pthread_exit(NULL);
	}

	if(rpStream == NULL) {
		printf("PC:\tUnable to receive stream to RP\n");
		pthread_exit(NULL);
	}

	lHeartbeat* sent = NULL;
	lResponse* unmatched = NULL;

	while(running) {
		char* cmd = stream_rcv_nblock(cmdStream, &len);

		if(cmd != NULL) {
			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
				running = 0;
			}

			free(cmd);
		}

		getSentHeartbeats(&sent, txStream);
		if(getReceivedMessages(rxStream, rpStream, &sent, &unmatched) == -1) {
			printf("malloc error in PC\n");
			pthread_exit(NULL);
		}

		if(sent != NULL) {
			lHeartbeat* cur = sent;

			struct timeval now;
			gettimeofday(&now, NULL);

			while(cur->next != NULL) {
				if(cur->timeSent.tv_sec > now.tv_sec || 
						now.tv_usec - cur->timeSent.tv_usec > 400000) {
						// If more than 400ms has passed, remove the
						// heartbeat from the list
					printf("Timedout heartbeat:\n");
					printHeartbeat(cur->h);

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

					lHeartbeat* x = cur->next;
					free(cur);
					cur = x;
				} else {
					cur = cur->next;
				}
			}

			if(cur->timeSent.tv_sec > now.tv_sec ||
					now.tv_usec - cur->timeSent.tv_usec > 400000) {
				printf("Timedout heartbeat:\n");
				printHeartbeat(cur->h);

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
		}

		if(unmatched != NULL) {
			lResponse* cur = unmatched;

			while(cur != NULL) {
				if(cur->counter == 0) {
					cur->counter = 1;
					cur = cur->next;
				} else {
					int removed = 0;
					lHeartbeat* current = sent;

					while(current != NULL) {
						if(cur->r->magic == current->h->magic) {
							printf("Heartbeat:\n");
							printHeartbeat(current->h);
							printf("matched with response:\n");
							printResponse(cur->r);

							if(cur->prev != NULL) {
								cur->prev->next = cur->next;
							}

							if(cur->next != NULL) {
								cur->next->prev = cur->prev;
							}

							if(cur == unmatched) {
								unmatched = cur->next;
							}

							free(cur->r);
							lResponse* x = cur->next;
							free(cur);
							cur = x;

							if(current->prev != NULL) {
								current->prev->next = current->next;
							}

							if(current->next != NULL) {
								current->next->prev = current->prev;
							}

							if(current == sent) {
								sent = current->next;
							}

							free(current->h);
							free(current);

							removed = 1;
							break;
						} else {
							current = current->next;
						}
					}

					if(removed == 0) {
						printf("Response unmatched:\n");
						printResponse(cur->r);

						if(cur->next != NULL) {
							cur->next->prev = cur->prev;
						}

						if(cur->prev != NULL) {
							cur->prev->next = cur->next;
						}

						if(cur == unmatched) {
							unmatched = cur->next;
						}

						lResponse* x = cur->next;
						free(cur->r);
						free(cur);
						cur = x;
					}
				}
			}
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
	pthread_exit(NULL);
}
