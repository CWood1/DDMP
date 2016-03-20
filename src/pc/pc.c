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
#include "response.h"
#include "api.h"
#include "commands.h"

#include <dhcpext/pc.h>
#include <dhcpext/common.h>

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

void* pcmain(void* ctSock) {
	int tx_sd, rx_sd, rp_sd, ct_sd;
	char running = 1;

	ct_sd = *((int*)ctSock);

	if((tx_sd = getSockFromSock(ct_sd)) == -1) {
		perror("PC");
		pthread_exit(NULL);
	}

	if((rx_sd = getSockFromSock(ct_sd)) == -1) {
		printf("PC:\nUnable to receive socket to RX\n");
		pthread_exit(NULL);
	}

	if((rp_sd = getSockFromSock(ct_sd)) == -1) {
		printf("PC:\nUnable to receive socket to RP\n");
		pthread_exit(NULL);
	}

	lHeartbeat* sent = NULL;
	lResponse* unmatched = NULL;

	while(running) {
		fd_set set;
		FD_ZERO(&set);

		FD_SET(tx_sd, &set);
		FD_SET(rx_sd, &set);
		FD_SET(ct_sd, &set);

		int largest = tx_sd;
		largest = (largest < rx_sd) ? rx_sd : largest;
		largest = (largest < ct_sd) ? ct_sd : largest;

		if(select(largest + 1, &set, NULL, NULL, NULL) == -1) {
			printf("Select error in PC\n");
			close(tx_sd);
			close(rx_sd);
			close(ct_sd);
			pthread_exit(NULL);
		}

		if(FD_ISSET(ct_sd, &set)) {
			switch(handleCommands(ct_sd)) {
				case 1:
					running = 0;
					break;
				case -1:
					printf("PC encountered an error when processing commands.\n");
					close(tx_sd);
					close(rx_sd);
					close(ct_sd);
					pthread_exit(NULL);
					break;
			}
		}

		if(FD_ISSET(tx_sd, &set)) {
			getSentHeartbeats(&sent, tx_sd);
		}

		if(FD_ISSET(rx_sd, &set)) {
			if(getReceivedMessages(rx_sd, rp_sd, &sent, &unmatched) == -1) {
				printf("malloc error in PC\n");
				close(tx_sd);
				close(rx_sd);
				close(ct_sd);
				pthread_exit(NULL);
			}
		}

		checkUnmatchedList(&unmatched, &sent);
		removeTimedoutHeartbeats(&sent);

	}

	freeHeartbeatList(sent);
	freeResponseList(unmatched);

	printf("pc shutting down.\n");
	close(tx_sd);
	close(rx_sd);
	close(rp_sd);
	close(ct_sd);
	pthread_exit(NULL);
}
