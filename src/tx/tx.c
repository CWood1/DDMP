#include "transmit.h"
#include "config.h"
#include "commands.h"

#include <dhcpext/tx.h>
#include <dhcpext/pc.h>
#include <dhcpext/common.h>

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
#include <sys/select.h>

void* txmain(void* ctSock) {
	int sd, ct_sd, pc_sd;
	struct sockaddr_in bcastaddr, directaddr;
	int flags = 0;
	char* str_bcastaddr, *str_directaddr;

	ct_sd = *((int*)ctSock);
	
	if(getConfig(ct_sd, &str_bcastaddr, &str_directaddr, &flags) == -1) {
		printf("TX:\tUnable to receive configuration.\n");
		pthread_exit(NULL);
	}

	if((pc_sd = getSockFromSock(ct_sd)) == -1) {
		printf("TX:\tUnable to receive socket to PC.\n");
		pthread_exit(NULL);
	}

	if((sd = setupSocket(NETWORKFLAGS_BCAST)) < 0) {
		perror("Error in setting up socket");
		pthread_exit(NULL);
	}

	if(createAddr(inet_addr(str_bcastaddr), &bcastaddr) == -1) {
		perror("Unable to create broadcast address");
		pthread_exit(NULL);
	}

	if(createAddr(inet_addr(str_directaddr), &directaddr) == -1) {
		perror("Unable to create direct address");
		pthread_exit(NULL);
	}

	free(str_bcastaddr);
	free(str_directaddr);

	while(1) {
		fd_set set;
		FD_ZERO(&set);

		FD_SET(ct_sd, &set);

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;
			// Timeout after 100ms

		if(select(ct_sd + 1, &set, NULL, NULL, &timeout) == -1) {
			printf("Select error in TX\n");
			close(sd);
			close(ct_sd);
			close(pc_sd);
			pthread_exit(NULL);
		}

		if(FD_ISSET(ct_sd, &set)) {
			switch(handleCommands(ct_sd)) {
				case 1:
					printf("tx shutting down.\n");
					close(sd);	
					close(ct_sd);
					close(pc_sd);
					pthread_exit(NULL);
					break;
				case 2:
					flags |= TXFLAGS_BCAST;
					break;
				case 3:
					flags &= ~TXFLAGS_BCAST;
					break;
				case -1:
					printf("TX encountered an error while parsing commands.\n");
					close(sd);
					close(ct_sd);
					close(pc_sd);
					pthread_exit(NULL);
					break;
			}
		}

		switch(sendHeartbeats(flags, sd, bcastaddr, directaddr, pc_sd)) {
			case 1:
				printf("Unable to send broadcast heartbeat.\n");
				close(sd);
				close(ct_sd);
				close(pc_sd);
				pthread_exit(NULL);
				break;
			case 2:
				printf("Unable to send direct heartbeat.\n");
				close(sd);
				close(ct_sd);
				close(pc_sd);
				pthread_exit(NULL);
				break;
		}
	}
}
