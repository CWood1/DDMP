#include "tx.h"
#include "transmit.h"
#include "config.h"
#include "network.h"
#include "flags.h"

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