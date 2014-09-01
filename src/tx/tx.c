#include "tx.h"
#include "transmit.h"
#include "../pc/pc.h"
#include "../common.h"
#include "../stream.h"
#include "../proto.h"

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

int setupSocket() {
	int sd;
	int broadcast = 1;

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	if(setsockopt(sd, SOL_SOCKET, SO_BROADCAST, (void*)&broadcast, sizeof(broadcast)) < 0) {
		return -1;
	}

	return sd;
}

int createAddr(char* addr, struct sockaddr_in* saddr) {
	memset(saddr, 0, sizeof(struct sockaddr_in));
	saddr->sin_family = AF_INET;
	saddr->sin_port = htons(PORT);

	if((saddr->sin_addr.s_addr = inet_addr(addr)) == (unsigned long)INADDR_NONE) {
		return 1;
	}

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

	if((sd = setupSocket()) < 0) {
		perror("Error in setting up socket");
		pthread_exit(NULL);
	}

	if(createAddr(str_bcastaddr, &bcastaddr) == 1) {
		perror("Unable to create broadcast address");
		pthread_exit(NULL);
	}

	if(createAddr(str_directaddr, &directaddr) == 1) {
		perror("Unable to create direct address");
		pthread_exit(NULL);
	}

	free(str_bcastaddr);
	free(str_directaddr);

	while(1) {
		switch(sendHeartbeats(flags, sd, bcastaddr, directaddr, pcStream)) {
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
