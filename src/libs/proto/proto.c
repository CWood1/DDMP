#include <dhcpext/proto.h>

#include <errno.h>
#include <string.h>

int isHeartbeat(char* c, size_t length) {
	if(length == 9 && c[0] == 0) {
		return TYPE_HEARTBEAT;
	} else if(length == 9 && c[0] == 1) {
		return TYPE_RESPONSE;
	} else {
		errno = EINVAL;
		return -1;
	}
}
