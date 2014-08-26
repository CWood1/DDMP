#include "rp.h"
#include "stream.h"
#include "common.h"
#include "proto.h"

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

void* rpmain(void* s) {
	int len;

	tStream* cmdStream = (tStream*) s;
	tStream** pPcStream = (tStream**)(stream_rcv(cmdStream, &len));

	if(len != sizeof(tStream*)) {
		printf("Error receiving RP/PC stream\n");
		pthread_exit(NULL);
	}

	tStream* pcStream = *pPcStream;
	free(pPcStream);

	int sd;
	struct sockaddr_in replyaddr;

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("RP - socket error\n");
		pthread_exit(NULL);
	}

	memset(&replyaddr, 0, sizeof(replyaddr));
	replyaddr.sin_family = AF_INET;
	replyaddr.sin_port = htons(PORT);

	while(1) {
		char* cmd = stream_rcv_nblock(cmdStream, &len);

		if(cmd != NULL) {
			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
				printf("rp shutting down\n");
				free(cmd);
				pthread_exit(NULL);
			}

			free(cmd);
		}

		heartbeat* h = (heartbeat*)(stream_rcv_nblock(pcStream, &len));

		if(h != NULL) {
			response* r = craftResponse(h);
			char* b = serializeResponse(r, &len);

			int rc = sendto(sd, b, len, 0,
				(struct sockaddr*)&replyaddr, sizeof(replyaddr));

			free(h);
			free(r);
			free(b);

			if(rc < 0) {
				perror("sendto error");
				close(sd);
				pthread_exit(NULL);
			}
		}
	}
}
