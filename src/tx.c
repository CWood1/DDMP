#include "tx.h"
#include "pc/pc.h"
#include "common.h"
#include "stream.h"
#include "proto.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

lHeartbeat* sendHeartbeat(int sd, struct sockaddr_in addr, tStream* pcStream, int flags) {
	int length;

	heartbeat* h = craftHeartbeat(flags);
	char* buffer = serializeHeartbeat(h, &length);

	int rc = sendto(sd, buffer, length, 0,
		(struct sockaddr*)&addr, sizeof(addr));
	free(buffer);

	if(rc < 0) {
		return NULL;
	}

	lHeartbeat* s = malloc(sizeof(lHeartbeat));

	if(s == NULL) {
		return NULL;
	}

	s->next = NULL;
	s->prev = NULL;
	s->h = h;
	s->addrv4 = addr.sin_addr.s_addr;
	gettimeofday(&(s->timeSent), NULL);

	stream_send(pcStream, (char*)s, sizeof(lHeartbeat));
	return s;
}

int sendHeartbeats(struct timeval* last, int flags, int sd, struct sockaddr_in bcastaddr,
		struct sockaddr_in directaddr, tStream* pcStream) {
	struct timeval now;
	gettimeofday(&now, NULL);

	if(last->tv_sec == 0 ||
			last->tv_sec < now.tv_sec || (now.tv_usec - last->tv_usec) >= 100000) {
		lHeartbeat* sent;

		if(flags & TXFLAGS_BCAST) {
			sent = sendHeartbeat(sd, bcastaddr, pcStream, flags);

			if(sent == NULL) {
				return 1;
			}

			free(sent);
		}

		sent = sendHeartbeat(sd, directaddr, pcStream, flags & ~TXFLAGS_BCAST);

		if(sent == NULL)
			return 2;

		*last = sent->timeSent;
		free(sent);
	}

	return 0;
}

int getConfig(tStream* cmdStream, char** str_bcastaddr, char** str_directaddr,
		int* flags) {
	int len;

	char* str_config = stream_rcv(cmdStream, &len);
	int* pFlags = (int*)(stream_rcv(cmdStream, &len));

	if(str_config == NULL)
		return 1;

	if(pFlags == NULL)
		return 1;

	char* tstr_bcastaddr = strtok(str_config, " ");
	char* tstr_directaddr = strtok(NULL, " ");

	if(tstr_bcastaddr == NULL)
		return 1;

	if(tstr_directaddr == NULL)
		return 1;

	*str_bcastaddr = malloc(strlen(tstr_bcastaddr) + 1);
	*str_directaddr = malloc(strlen(tstr_directaddr) + 1);

	if(*str_bcastaddr == NULL)
		return 1;

	if(*str_directaddr == NULL)
		return 1;

	strcpy(*str_bcastaddr, tstr_bcastaddr);
	strcpy(*str_directaddr, tstr_directaddr);

	free(str_config);

	*flags = *pFlags;
	free(pFlags);

	return 0;
}

void* txmain(void* stream) {
	int sd, rc, len;
	struct sockaddr_in bcastaddr, directaddr;
	int flags = 0;
	char* str_bcastaddr, *str_directaddr;

	tStream* cmdStream = (tStream*) stream;

	if(getConfig(cmdStream, &str_bcastaddr, &str_directaddr, &flags) == 1) {
		printf("TX:\tUnable to receive configuration.\n");
		pthread_exit(NULL);
	}

	tStream* pcStream = getStreamFromStream(cmdStream);

	if(pcStream == NULL) {
		printf("TX:\tUnable to receive stream to PC\n");
		pthread_exit(NULL);
	}

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket error");
		pthread_exit(NULL);
	}

	int broadcast = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_BROADCAST, (void*)&broadcast, sizeof(broadcast)) < 0) {
		perror("Unable to set broadcast");
		pthread_exit(NULL);
	}

	memset(&bcastaddr, 0, sizeof(bcastaddr));
	bcastaddr.sin_family = AF_INET;
	bcastaddr.sin_port = htons(PORT);

	if((bcastaddr.sin_addr.s_addr = inet_addr(str_bcastaddr)) == (unsigned long)INADDR_NONE) {
		perror("Unable to broadcast");
		pthread_exit(NULL);
	}
	free(str_bcastaddr);

	memset(&directaddr, 0, sizeof(directaddr));
	directaddr.sin_family = AF_INET;
	directaddr.sin_port = htons(PORT);

	if((directaddr.sin_addr.s_addr = inet_addr(str_directaddr))
			== (unsigned long) INADDR_NONE) {
		perror("Unable to send direct heartbeat");
		pthread_exit(NULL);
	}
	free(str_directaddr);

	struct timeval last;
	last.tv_sec = 0;

	while(1) {
		switch(sendHeartbeats(&last, flags, sd, bcastaddr, directaddr, pcStream)) {
			case 1:
				printf("Unable to send broadcast heartbeat.\n");
				close(sd);
				pthread_exit(NULL);
				break;
			case 2:
				printf("Unable to send direct heartbeat.\n");
				close(sd);
				pthread_exit(NULL);
				break;
		}

		char* cmd = stream_rcv_nblock(cmdStream, &len);

		if(cmd != NULL) {
			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
				free(cmd);
				printf("tx shutting down.\n");
				close(sd);
				pthread_exit(NULL);
			} else if(strcmp(t, "bcastflag")) {
				t = strtok(NULL, " ");

				if(t == "1") {
					flags |= TXFLAGS_BCAST;
				} else {
					flags &= ~TXFLAGS_BCAST;
				}
			}

			free(cmd);
		}
	}
}
