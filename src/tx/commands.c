#include "commands.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>

int handleCommands(int sd) {
	size_t size;
	ssize_t rc;
	char* cmd;

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return -1;
	}

	if(size == 0) {
		errno = ENOMSG;
		return -1;
	}

	cmd = malloc(size);

	if(cmd == NULL) {
		return -1;
	}

	rc = recv(sd, cmd, size, 0);

	if(rc < 0) {
		return -1;
	}

	char* s = strtok(cmd, " ");

	if(strcmp(s, "shutdown") == 0) {
		free(cmd);
		return 1;
	}

	if(strcmp(s, "setbcast") == 0) {
		s = strtok(NULL, " ");

		if(strcmp(s, "1") == 0) {
			free(cmd);
			return 2;
		}

		if(strcmp(s, "0") == 0) {
			free(cmd);
			return 3;
		}
	}

	return 0;
}
