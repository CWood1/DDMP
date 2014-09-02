#include <dhcpext/rp.h>
#include <dhcpext/common.h>
#include <dhcpext/proto.h>
#include <dhcpext/stream.h>

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
	int sd;
	struct sockaddr_in replyaddr;

	tStream* cmdStream = (tStream*) s;
	tStream* pcStream = getStreamFromStream(cmdStream);

	if(pcStream == NULL) {
		printf("RP: Unable to receive stream to PC\n");
		pthread_exit(NULL);
	}

	if((sd = setupSocket(0)) < 0) {
		printf("RP: Unable to set up socket\n");
		pthread_exit(NULL);
	}

	if(createAddr(htonl(INADDR_ANY), &replyaddr) == 1) {
		printf("RP: Unable to set up reply address\n");
		pthread_exit(NULL);
	}

	while(1) {
		unsigned int len;
		char* cmd = stream_rcv_nblock(cmdStream, &len);

		if(cmd != NULL) {
			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
				printf("rp shutting down\n");
				free(cmd);
				close(sd);
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

			freeHeartbeat(h);
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
