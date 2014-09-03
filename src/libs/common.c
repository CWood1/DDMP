#include <dhcpext/common.h>
#include <dhcpext/stream.h>

#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

tStream* getStreamFromStream(tStream* cmdStream) {
	unsigned int len;
	tStream** new = (tStream**)(stream_rcv(cmdStream, &len));

	if(len != sizeof(tStream*)) {
		return NULL;
	}

	tStream* s = *new;
	free(new);

	return s;
}

tStream* getStreamFromSock(int sd) {
	size_t size;
	ssize_t rc;
	tStream** new;
	tStream* ret;
	fd_set set;

	FD_ZERO(&set);
	FD_SET(sd, &set);

	if(select(sd + 1, &set, NULL, NULL, NULL) == -1) {
		return NULL;
	}

	if(ioctl(sd, FIONREAD, &size) != 0) {
		return NULL;
	}

	if(size != sizeof(tStream*)) {
		return NULL;
	}

	new = malloc(size);

	if(new == NULL) {
		return NULL;
	}

	rc = recv(sd, (char*)new, size, 0);

	if(rc < 0) {
		return NULL;
	}

	ret = *new;
	free(new);

	return ret;
}
