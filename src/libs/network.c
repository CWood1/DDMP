#include <dhcpext/common.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int setupSocket(int flags) {
	int sd;
	int broadcast = 1;

	if((sd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	if(flags & NETWORKFLAGS_BCAST) {
		if(setsockopt(sd, SOL_SOCKET, SO_BROADCAST,
				(void*)&broadcast, sizeof(broadcast)) < 0) {
			return -1;
		}
	}

	return sd;
}

int createAddr(uint32_t addr, struct sockaddr_in* saddr) {
	if(saddr == NULL) {
		errno = EINVAL;
		return 1;
	}

	memset(saddr, 0, sizeof(struct sockaddr_in));
	saddr->sin_family = AF_INET;
	saddr->sin_port = htons(PORT);

	if((saddr->sin_addr.s_addr = addr) == (unsigned long)INADDR_NONE) {
		return 1;
	}

	return 0;
}
