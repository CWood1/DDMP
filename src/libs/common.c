#include <dhcpext/common.h>
#include <dhcpext/stream.h>

#include <stdlib.h>

tStream* getStreamFromStream(tStream* cmdStream) {
	int len;
	tStream** new = (tStream**)(stream_rcv(cmdStream, &len));

	if(len != sizeof(tStream*)) {
		return NULL;
	}

	tStream* s = *new;
	free(new);

	return s;
}
