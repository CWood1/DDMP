#include <dhcpext/proto.h>

int isHeartbeat(char* c, unsigned int length) {
	if(length == 9 && c[0] == 0) {
		return 1;
	} else if(length == 9 && c[0] == 1) {
		return 0;
	} else {
		return -1;
	}
}
