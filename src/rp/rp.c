/**
    This file is part of mesh-dhcp-ext, the Mesh Network DHCP Extensions protocol.

    mesh-dhcp-ext is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    mesh-dhcp-ext is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with mesh-dhcp-ext.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2015 Connor Wood <connorwood71@gmail.com>.
*/

#include "heartbeat.h"
#include "commands.h"

#include <dhcpext/rp.h>
#include <dhcpext/common.h>
#include <dhcpext/proto.h>

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

	if(createAddr(htonl(INADDR_ANY), &replyaddr) == -1) {
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

			if(r == NULL) {
				printf("RP:\tError while crafting response\n");
				close(sd);
				close(pc_sd);
				close(ct_sd);
				pthread_exit(NULL);
			}

			char* b = serializeResponse(r, &len);

			if(b == NULL) {
				printf("RP:\nError serializing response\n");
				close(sd);
				close(pc_sd);
				close(ct_sd);
				pthread_exit(NULL);
			}

			ssize_t rc = sendto(sd, b, len, 0,
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
