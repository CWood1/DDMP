#include "commands.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

int handleCommands(int sd) {
	size_t size;
	char* cmd;

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return -1;
	}

	if(size == 0) {
		return -1;
	}

	cmd = malloc(size);

	if(cmd == NULL) {
		return -1;
	}

	if(recv(sd, cmd, size, 0) < 0) {
		return -1;
	}

	char* s = strtok(cmd, " ");

	if(strcmp(s, "shutdown") == 0) {
		free(cmd);
		return 1;
	}

	free(cmd);
	return 0;
}
