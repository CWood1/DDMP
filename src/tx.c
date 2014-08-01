#include "tx.h"
#include "common.h"
#include "stream.h"

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
	char buffer[100];
	int bcastFlag = 0;

	tStream* cmdStream = (tStream*) stream;
	char* str_bcastaddr = stream_rcv(cmdStream);
	char* str_directaddr = stream_rcv(cmdStream);
	char* bcastActive = stream_rcv(cmdStream);
		// Get the options from CT

	if(strcmp(bcastActive, "1") == 0) {
		bcastFlag = 1;
	}

	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, "Heartbeat", strlen("Heartbeat"));

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
		if(bcastFlag == 1) {
			rc = sendto(sd, (char*)buffer, sizeof(buffer), 0,
				(struct sockaddr*)&bcastaddr, sizeof(bcastaddr));

			if(rc < 0) {
				perror("Unable to send broadcast heartbeat");
				close(sd);
				pthread_exit(NULL);
			}
		}

		rc = sendto(sd, (char*) buffer, sizeof(buffer), 0, (struct sockaddr*)&directaddr,
			sizeof(directaddr));

		if(rc < 0) {
			perror("Unable to send direct heartbeat");
			close(sd);
			pthread_exit(NULL);
		}

		char* cmd = stream_rcv_nblock(cmdStream);
		if(cmd != NULL) {
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
