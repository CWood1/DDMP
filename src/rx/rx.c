#include "../common.h"
#include "../stream.h"
#include "../proto.h"

#include <dhcpext/rx.h>
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

int receive(struct sockaddr_in replyaddr, int sd, tStream* pcStream) {
	char buffer[100];
	int rc;

	int addrlen = sizeof(replyaddr);
	rc = recvfrom(sd, (char*)buffer, sizeof(buffer), MSG_DONTWAIT,
		(struct sockaddr*)&replyaddr, &addrlen);

	if(rc < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
		return 1;
	} else if(rc >= 0) {
		message* m = malloc(sizeof(message));

		if(m == NULL) {
			return 1;
		}

		m->buffer = malloc(rc);

		if(m->buffer == NULL) {
			return 1;
		}

		memcpy(m->buffer, (void*)buffer, rc);
		m->addrv4 = replyaddr.sin_addr.s_addr;
		m->bufferSize = rc;

		stream_send(pcStream, (char*)m, sizeof(message));
		free(m);
	}

	return 0;
}

void* rxmain(void* stream) {
	int sd;
	struct sockaddr_in selfaddr, replyaddr;

	tStream* cmdStream = (tStream*)stream;
	tStream* pcStream = getStreamFromStream(cmdStream);

	if(pcStream == NULL) {
		printf("RX: Error setting up stream to PC\n");
		pthread_exit(NULL);
	}

	if((sd = setupSocket(0)) < 0) {
		perror("Heartbeat receive - socket error.");
		pthread_exit(NULL);
	}

	if(createAddr(htonl(INADDR_ANY), &selfaddr) == 1) {
		printf("RX: unable to initialize self address\n");
		pthread_exit(NULL);
	}

	if(createAddr(htonl(INADDR_ANY), &replyaddr) == 1) {
		printf("RX: unable to initialize reply address\n");
		pthread_exit(NULL);
	}

	if(bind(sd, (struct sockaddr*)&selfaddr, sizeof(selfaddr)) < 0) {
		perror("Heartbeat receive - bind error.");
		close(sd);
		pthread_exit(NULL);
	}

	while(1) {
		int len;

		if(receive(replyaddr, sd, pcStream) == 1) {
			printf("Error receiving traffic from network\n");
			pthread_exit(NULL);
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