#include "receive.h"
#include "commands.h"

#include <dhcpext/rx.h>
#include <dhcpext/pc.h>
#include <dhcpext/common.h>
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
#include <sys/select.h>

void* rxmain(void* ctSock) {
	int sd, ct_sd, pc_sd;
	struct sockaddr_in selfaddr, replyaddr;

	ct_sd = *((int*)ctSock);

	if((pc_sd = getSockFromSock(ct_sd)) < 0) {
		printf("RX: Error setting up socket to PC\n");
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
		close(ct_sd);
		close(pc_sd);
		pthread_exit(NULL);
	}

	while(1) {
		fd_set set;
		FD_ZERO(&set);

		FD_SET(sd, &set);
		FD_SET(ct_sd, &set);

		if(select(((sd > ct_sd) ? sd : ct_sd) + 1, &set, NULL, NULL, NULL) == -1) {
			printf("Select error in RX\n");
			close(sd);
			close(ct_sd);
			close(pc_sd);
			pthread_exit(NULL);
		}

		if(FD_ISSET(sd, &set)) {
			if(receive(replyaddr, sd, pc_sd) == 1) {
				printf("Error receiving traffic from network\n");
				pthread_exit(NULL);
			}
		}

		if(FD_ISSET(ct_sd, &set)) {
			switch(handleCommands(ct_sd)) {
				case 1:
					printf("rx shutting down.\n");
					close(ct_sd);
					close(sd);
					close(pc_sd);
					pthread_exit(NULL);
				case -1:
					printf("RX encountered an error when processing commands.\n");
					close(ct_sd);
					close(sd);
					close(pc_sd);
					pthread_exit(NULL);
			}
		}
	}
}
