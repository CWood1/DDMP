#include "rx.h"
#include "common.h"
#include "stream.h"
#include "proto.h"

#include <dhcpext/pc.h>

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

void* rxmain(void* stream) {
	int sd, rc, len;
	struct sockaddr_in selfaddr, senderaddr, replyaddr;
	char buffer[100];

	tStream* cmdStream = (tStream*)stream;
	tStream* pcStream = getStreamFromStream(cmdStream);

	if(pcStream == NULL) {
		printf("RX: Error setting up stream to PC\n");
		pthread_exit(NULL);
	}

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Heartbeat receive - socket error.");
		pthread_exit(NULL);
	}

	memset(&selfaddr, 0, sizeof(selfaddr));
	selfaddr.sin_family = AF_INET;
	selfaddr.sin_port = htons(PORT);
	selfaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	memset(&replyaddr, 0, sizeof(replyaddr));
	replyaddr.sin_family = AF_INET;
	replyaddr.sin_port = htons(PORT);

	if((rc = bind(sd, (struct sockaddr*)&selfaddr, sizeof(selfaddr))) < 0) {
		perror("Heartbeat receive - bind error.");
		close(sd);
		pthread_exit(NULL);
	}

	while(1) {
		int senderaddrlen = sizeof(senderaddr);
		rc = recvfrom(sd, (char*)buffer, sizeof(buffer), MSG_DONTWAIT,
			(struct sockaddr*)&senderaddr, &senderaddrlen);

		if(rc < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("recvfrom error");
			close(sd);
			pthread_exit(NULL);
		} else if(rc >= 0) {
			message* m = malloc(sizeof(message));

			if(m == NULL) {
				printf("malloc error in rc\n");
				pthread_exit(NULL);
			}

			m->buffer = malloc(rc);

			if(m->buffer == NULL) {
				printf("malloc error in rc\n");
				pthread_exit(NULL);
			}

			memcpy(m->buffer, (void*)buffer, rc);
			m->addrv4 = senderaddr.sin_addr.s_addr;
			m->bufferSize = rc;
			
			stream_send(pcStream, (char*)m, sizeof(message));
			free(m);
		}

		char* cmd = stream_rcv_nblock(cmdStream, &len);

		if(cmd != NULL) {
			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
				free(cmd);
				printf("rx shutting down.\n");
				close(sd);
				pthread_exit(NULL);
			}

			free(cmd);
		}
	}
}
