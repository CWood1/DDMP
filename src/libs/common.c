#include <dhcpext/common.h>

#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

int getSockFromSock(int sd) {
	size_t size;
	int ret;
	fd_set set;

	FD_ZERO(&set);
	FD_SET(sd, &set);

	if(select(sd + 1, &set, NULL, NULL, NULL) == -1) {
		return -1;
	}

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return -1;
	}

	if(size != sizeof(int)) {
		return -1;
	}

	if(recv(sd, &ret, size, 0) < 0) {
		return -1;
	}

	return ret;
}

