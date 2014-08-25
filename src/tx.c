#include "tx.h"
#include "pc.h"
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

void* txmain(void* stream) {
	int sd, rc, len;
	struct sockaddr_in bcastaddr, directaddr;
	int bcastFlag = 0;

	tStream* cmdStream = (tStream*) stream;

	char* str_bcastaddr = stream_rcv(cmdStream, &len);
	char* str_directaddr = stream_rcv(cmdStream, &len);
	char* bcastActive = stream_rcv(cmdStream, &len);

	tStream** pPcStream = (tStream**)(stream_rcv(cmdStream, &len));

	if(len != sizeof(tStream*)) {
		printf("Error setting up TX/PC stream\n");
		pthread_exit(NULL);
	}

	tStream* pcStream = *pPcStream;
	free(pPcStream);

	if(strcmp(bcastActive, "1") == 0) {
		bcastFlag = 1;
	}

	free(bcastActive);

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

	while(1) {
		heartbeat* h;
		char* buffer;
		int length;

		if(bcastFlag == 1) {
			h = craftHeartbeat(1);
			buffer = serializeHeartbeat(h, &length);

			rc = sendto(sd, buffer, length, 0,
				(struct sockaddr*)&bcastaddr, sizeof(bcastaddr));
			free(buffer);

			lHeartbeat* s = malloc(sizeof(lHeartbeat));

			if(s == NULL) {
				printf("malloc error in tx\n");
				close(sd);
				pthread_exit(NULL);
			}

			s->next = NULL;
			s->prev = NULL;
			s->h = h;
			s->addrv4 = bcastaddr.sin_addr.s_addr;
			gettimeofday(&(s->timeSent), NULL);

			stream_send(pcStream, (char*)s, sizeof(lHeartbeat));
			free(s);

			if(rc < 0) {
				// At some point, inform CT about this
				perror("Unable to send broadcast heartbeat");
				close(sd);
				pthread_exit(NULL);
			}
		}

		h = craftHeartbeat(0);
		buffer = serializeHeartbeat(h, &length);

		rc = sendto(sd, buffer, length, 0, (struct sockaddr*)&directaddr,
			sizeof(directaddr));
		free(buffer);

		lHeartbeat* s = malloc(sizeof(lHeartbeat));

		if(s == NULL) {
			printf("malloc error in tx\n");
			close(sd);
			pthread_exit(NULL);
		}

		s->next = NULL;
		s->prev = NULL;
		s->h = h;
		s->addrv4 = directaddr.sin_addr.s_addr;
		gettimeofday(&(s->timeSent), NULL);

		stream_send(pcStream, (char*)s, sizeof(lHeartbeat));
		free(s);

		if(rc < 0) {
			perror("Unable to send direct heartbeat");
			close(sd);
			pthread_exit(NULL);
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
					bcastFlag = 1;
				} else {
					bcastFlag = 0;
				}
			}

			free(cmd);
		}

		usleep(100000);
			// 100 milliseconds, or should be :)
	}
}
