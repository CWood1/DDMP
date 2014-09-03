#include "heartbeat.h"
#include "commands.h"

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
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

void* rpmain(void* ctSock) {
	int sd, ct_sd, pc_sd;
	struct sockaddr_in replyaddr;

	ct_sd = *((int*)ctSock);

	if((pc_sd = getSockFromSock(ct_sd)) < 0) {
		printf("RP: Unable to receive socket to PC.\n");
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
		fd_set set;
		FD_ZERO(&set);

		FD_SET(ct_sd, &set);
		FD_SET(pc_sd, &set);

		if(select(((ct_sd > pc_sd) ? ct_sd : pc_sd) + 1, &set, NULL, NULL, NULL) == -1) {
			printf("Select error in RP\n");
			close(sd);
			close(ct_sd);
			close(pc_sd);
			pthread_exit(NULL);
		}

		if(FD_ISSET(ct_sd, &set)) {
			switch(handleCommands(ct_sd)) {
				case 1:
					printf("rp shutting down.\n");
					close(sd);
					close(ct_sd);
					close(pc_sd);
					pthread_exit(NULL);
					break;
				case -1:
					printf("RP:\tError while processing commands.\n");
					close(sd);
					close(ct_sd);
					close(pc_sd);
					pthread_exit(NULL);
					break;
			}
		}

		if(FD_ISSET(pc_sd, &set)) {
			heartbeat* h;
			unsigned int len;

			h = getHeartbeatFromSock(pc_sd);

			if(h == NULL) {
				printf("RP:\tError while fetching heartbeat from PC\n");
				close(sd);
				close(pc_sd);
				close(ct_sd);
				pthread_exit(NULL);
			}

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
				close(pc_sd);
				close(ct_sd);
				pthread_exit(NULL);
			}
		}
	}
}
