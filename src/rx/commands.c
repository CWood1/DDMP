#include "commands.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

int handleCommands(int sd) {
	size_t size;
	ssize_t rc;
	char* cmd;

	if(ioctl(sd, FIONREAD, &size) != 0) {	
		return 2;
	}

	if(size == 0) {	
		return 2;
	}

	cmd = malloc(size);

	if(cmd == NULL) {
		return 2;
	}

	rc = recv(sd, cmd, size, 0);

	if(rc < 0) {
		return 2;
	}

	char* s = strtok(cmd, " ");

	if(strcmp(s, "shutdown") == 0) {
		free(cmd);
		return 1;
	}

	free(cmd);
	return 0;
}
