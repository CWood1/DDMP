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

tStream* getStreamFromStream(tStream* cmdStream) {
	int len;
	tStream** new = (tStream**)(stream_rcv(cmdStream, &len));

	if(len != sizeof(tStream*)) {
		return NULL;
	}

	tStream* s = *new;
	free(new);

	return s;
}

void getSentHeartbeats(lHeartbeat** sent, tStream* txStream) {
	int len;
	lHeartbeat* next = (lHeartbeat*)(stream_rcv_nblock(txStream, &len));

	while(next != NULL) {
		if(*sent == NULL) {
			*sent = next;

			next->prev = NULL;
			next->next = NULL;
		} else {
			lHeartbeat* cur = *sent;

			while(cur->next != NULL) {
				cur = cur->next;
			}

			cur->next = next;
			next->prev = cur;
			next->next = NULL;
		}

		struct in_addr addrv4;
		addrv4.s_addr = next->addrv4;

		printf("Heartbeat sent(%s):\n", inet_ntoa(addrv4));
		printHeartbeat(next->h);

		next = (lHeartbeat*)(stream_rcv_nblock(txStream, &len));
	}
}

void handleReceivedHeartbeat(heartbeat* h, struct in_addr addrv4, tStream* rpStream) {
	printf("Heartbeat received (%s):\n", inet_ntoa(addrv4));
	printHeartbeat(h);
	stream_send(rpStream, (char*)h, sizeof(heartbeat));
}

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

		message* m = (message*)(stream_rcv_nblock(rxStream, &len));

		while(m != NULL) {
			struct in_addr addrv4;
			addrv4.s_addr = m->addrv4;

			if(isHeartbeat(m->buffer, m->bufferSize)) {
				heartbeat* h = deserializeHeartbeat(m->buffer, m->bufferSize);
				handleReceivedHeartbeat(h, addrv4, rpStream);
				free(h);
			} else {
				printf("Response received (%s):\n",
					inet_ntoa(addrv4));

				response* r = deserializeResponse(m->buffer, m->bufferSize);
				printResponse(r);

				if(sent != NULL) {
					lHeartbeat* cur = sent;

					while(cur->h->magic != r->magic) {
						if(cur->next != NULL) {
							cur = cur->next;
						} else {
							break;
						}
					}

					if(cur->h->magic == r->magic) {
						printf("Response matches:\n");
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

						free(r);
					} else if(handleUnmatchedResponse(&unmatched, r) == NULL) {
						printf("malloc error in PC\n");
						pthread_exit(NULL);
					}
				} else if(handleUnmatchedResponse(&unmatched, r) == NULL) {
					printf("malloc error in PC\n");
					pthread_exit(NULL);
				}
	 		}

			free(m->buffer);
			free(m);
			m = (message*)(stream_rcv_nblock(rxStream, &len));
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
