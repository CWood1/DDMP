#include "send.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

void sendHeartbeat() {
	int sd, rc;
	struct sockaddr_in selfaddr, receiveraddr;
	char addrToSendTo[] = "10.255.255.255";
	char buffer[100];

	struct hostent* hostp;
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, "Heartbeat", strlen("Heartbeat"));

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket error");
		exit(-1);
	}

	int broadcast = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_BROADCAST, (void*)&broadcast, sizeof(broadcast)) < 0) {
		perror("Unable to set broadcast");
		exit(-1);
	}

	memset(&receiveraddr, 0, sizeof(receiveraddr));
	receiveraddr.sin_family = AF_INET;
	receiveraddr.sin_port = htons(PORT);

	if((receiveraddr.sin_addr.s_addr = inet_addr(addrToSendTo)) == (unsigned long)INADDR_NONE) {
		perror("Unable to broadcast");
		exit(-1);
	}

	rc = sendto(sd, (char*)buffer, sizeof(buffer), 0, (struct sockaddr*)&receiveraddr,
		sizeof(receiveraddr));

	if(rc < 0) {
		perror("sendto error");
		close(sd);
		exit(-1);
	}

	int receiveraddrlen = sizeof(receiveraddr);
	rc = recvfrom(sd, (char*)buffer, sizeof(buffer), 0, 
		(struct sockaddr*)&receiveraddr, &receiveraddrlen);

	if(rc < 0) {
		perror("recvfrom error");
		close(sd);
		exit(-1);
	}

	printf("Heartbeat reply received (%s):\n\t%s\n", inet_ntoa(receiveraddr.sin_addr),
		buffer);

	close(sd);
	exit(0);
}
