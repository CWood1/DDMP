#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "recv.h"
#include "send.h"
#include "dhcplease.h"

int main(int argc, char** argv) {
	if(argc != 2) {
		printf("Incorrect command.\n");
			// TODO: Write usage here
	} else {
		if(strcmp(argv[1], "send") == 0) {
			sendHeartbeat();
		} else if(strcmp(argv[1], "lease") == 0) {
			initDhcpLease();
		} else {
			recvHeartbeats();
		}
	}

	return 0;
}
