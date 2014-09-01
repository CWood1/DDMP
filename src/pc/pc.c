#include "heartbeat.h"
#include "response.h"
#include "api.h"
#include "../stream.h"
#include "../common.h"

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

void* pcmain(void* s) {
	int len;
	char running = 1;

	tStream* cmdStream = (tStream*) s;
	tStream* txStream = getStreamFromStream(cmdStream);
	tStream* rxStream = getStreamFromStream(cmdStream);
	tStream* rpStream = getStreamFromStream(cmdStream);

	if(txStream == NULL) {
		printf("PC:\tUnable to receive stream to TX\n");
		pthread_exit(NULL);
	}

	if(rxStream == NULL) {
		printf("PC:\tUnable to receive stream to RX\n");
		pthread_exit(NULL);
	}

	if(rpStream == NULL) {
		printf("PC:\tUnable to receive stream to RP\n");
		pthread_exit(NULL);
	}

	lHeartbeat* sent = NULL;
	lResponse* unmatched = NULL;

	while(running) {
		char* cmd = stream_rcv_nblock(cmdStream, &len);

		if(cmd != NULL) {
			char* t = strtok(cmd, " ");

			if(strcmp(t, "shutdown") == 0) {
				running = 0;
			}

			free(cmd);
		}

		getSentHeartbeats(&sent, txStream);
		if(getReceivedMessages(rxStream, rpStream, &sent, &unmatched) == -1) {
			printf("malloc error in PC\n");
			pthread_exit(NULL);
		}
		checkUnmatchedList(&unmatched, &sent);
		removeTimedoutHeartbeats(&sent);

	}

	freeHeartbeatList(sent);
	freeResponseList(unmatched);

	printf("pc shutting down.\n");
	pthread_exit(NULL);
}
