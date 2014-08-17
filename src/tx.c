#include "tx.h"
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
	int sd, rc;
	struct sockaddr_in bcastaddr, directaddr;
	int bcastFlag = 0;

	tStream* cmdStream = (tStream*) stream;

	char* str_bcastaddr = malloc(stream_wait_full(cmdStream));
	stream_rcv(cmdStream, 0, str_bcastaddr);
	char* str_directaddr = malloc(stream_wait_full(cmdStream));
	stream_rcv(cmdStream, 0, str_directaddr);
	char* bcastActive = malloc(stream_wait_full(cmdStream));
	stream_rcv(cmdStream, 0, bcastActive);
		// Get the options from CT
	tStream** pPcStream = malloc(stream_wait_full(cmdStream));
	stream_rcv(cmdStream, 0, (char*)pPcStream);

	tStream* pcStream = *pPcStream;
	free(pPcStream);

	if(strcmp(bcastActive, "1") == 0) {
		bcastFlag = 1;
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

	memset(&directaddr, 0, sizeof(directaddr));
	directaddr.sin_family = AF_INET;
	directaddr.sin_port = htons(PORT);

	if((directaddr.sin_addr.s_addr = inet_addr(str_directaddr))
			== (unsigned long) INADDR_NONE) {
		perror("Unable to send direct heartbeat");
		pthread_exit(NULL);
	}

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

			stream_send(pcStream, h, sizeof(heartbeat*));

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

		stream_send(pcStream, h, sizeof(heartbeat*));

		if(rc < 0) {
			perror("Unable to send direct heartbeat");
			close(sd);
			pthread_exit(NULL);
		}

		int size = stream_length(cmdStream);
		if(size != NULL) {
			char* cmd = malloc(stream_wait_full(cmdStream));
			stream_rcv(cmdStream, 0, cmd);

			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
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
		}

		usleep(100000);
			// 100 milliseconds, or should be :)
	}
}
