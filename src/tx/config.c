#include "config.h"

#include <dhcpext/stream.h>

#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

int getConfig(int sd, char** str_bcastaddr, char** str_directaddr,
		int* flags) {
	size_t size;
	ssize_t rc;
	fd_set set;

	FD_ZERO(&set);
	FD_SET(sd, &set);

	if(select(sd + 1, &set, NULL, NULL, NULL) == -1) {
		return 1;
	}

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return 1;
	}

	char* str_config = malloc(size);

	if(str_config == NULL) {
		return 1;
	}

	rc = recv(sd, str_config, size, 0);

	if(rc < 0) {
		return 1;
	}

	FD_ZERO(&set);
	FD_SET(sd, &set);

	if(select(sd + 1, &set, NULL, NULL, NULL) == -1) {
		return 1;
	}

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return 1;
	}

	int* pFlags = malloc(size);

	if(pFlags == NULL) {
		return 1;
	}

	rc = recv(sd, (char*)pFlags, size, 0);

	if(rc < 0) {
		return 1;
	}

	char* tstr_bcastaddr = strtok(str_config, " ");
	char* tstr_directaddr = strtok(NULL, " ");

	if(tstr_bcastaddr == NULL)
		return 1;

	if(tstr_directaddr == NULL)
		return 1;

	*str_bcastaddr = malloc(strlen(tstr_bcastaddr) + 1);
	*str_directaddr = malloc(strlen(tstr_directaddr) + 1);

	if(*str_bcastaddr == NULL)
		return 1;

	if(*str_directaddr == NULL)
		return 1;

	strcpy(*str_bcastaddr, tstr_bcastaddr);
	strcpy(*str_directaddr, tstr_directaddr);

	free(str_config);

	*flags = *pFlags;
	free(pFlags);

	return 0;
}
