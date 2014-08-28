#include "pc.h"
#include "heartbeat.h"
#include "response.h"
#include "api.h"
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
		removeTimedoutHeartbeats(&sent);

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
