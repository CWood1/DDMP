#include "rx.h"
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
#include <pthread.h>
#include <errno.h>

void* rxmain(void* stream) {
	int sd, rc;
	struct sockaddr_in selfaddr, senderaddr;
		// Structs for the sender of the heartbeat, and receiver of the heartbeat

	char buffer[100];
		// TODO: Clean up size here, to maximum needed size

	tStream* cmdStream = (tStream*)stream;
		// We don't need any params from this to start with

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Heartbeat receive - socket error.");
		pthread_exit(NULL);
	}

	memset(&selfaddr, 0, sizeof(selfaddr));
	selfaddr.sin_family = AF_INET;
	selfaddr.sin_port = htons(PORT);
	selfaddr.sin_addr.s_addr = htonl(INADDR_ANY);

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
			if(isHeartbeatOrResponse((char*)buffer) == 0) {
				printf("Heartbeat received (%s):\n",
					inet_ntoa(senderaddr.sin_addr));

				heartbeat* h = deserializeHeartbeat((char*)buffer);
				printHeartbeat(h);
				free(h->s);
				free(h);

				int size;

				response* r = craftResponse(h);
				char* b = serializeResponse(r, &size);

				rc = sendto(sd, b, size, 0,
					(struct sockaddr*)&senderaddr, senderaddrlen);

				free(r);
				free(b);

				if(rc < 0) {
					perror("sendto error");
					close(sd);
					pthread_exit(NULL);
				}
			} else {
				printf("Response received (%s):\n",
					inet_ntoa(senderaddr.sin_addr));

				response* r = deserializeResponse((char*)buffer);
				printResponse(r);
				free(r->s);
				free(r);
			}
		}

		char* cmd = stream_rcv_nblock(cmdStream);
		if(cmd != NULL) {
			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
				printf("rx shutting down.\n");
				close(sd);
				pthread_exit(NULL);
			}
		}
	}
}
