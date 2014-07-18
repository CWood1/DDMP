#include "recv.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void recvHeartbeats() {
	int sd, rc;
	struct sockaddr_in selfaddr, senderaddr;
		// Structs for the sender of the heartbeat, and receiver of the heartbeat

	char buffer[100];
		// TODO: Clean up size here, to maximum needed size

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Heartbeat receive - socket error.");
		exit(-1);
	}

	memset(&selfaddr, 0, sizeof(selfaddr));
	selfaddr.sin_family = AF_INET;
	selfaddr.sin_port = htons(PORT);
	selfaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if((rc = bind(sd, (struct sockaddr*)&selfaddr, sizeof(selfaddr))) < 0) {
		perror("Heartbeat receive - bind error.");
		close(sd);
		exit(-1);
	}

	int senderaddrlen = sizeof(senderaddr);
	rc = recvfrom(sd, (char*)buffer, sizeof(buffer), 0, (struct sockaddr*)&senderaddr,
		&senderaddrlen);

	if(rc < 0) {
		perror("recvfrom error");
		close(sd);
		exit(-1);
	}

	printf("Heartbeat received (%s):\n\t%s\n", inet_ntoa(senderaddr.sin_addr), buffer);

	rc = sendto(sd, (char*)buffer, sizeof(buffer), 0, (struct sockaddr*)&senderaddr,
		senderaddrlen);
	if(rc < 0) {
		perror("sendto error");
		close(sd);
		exit(-1);
	}

	close(sd);
}
