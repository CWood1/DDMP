#include "config.h"

#include <dhcpext/stream.h>

#include <stdlib.h>
#include <string.h>

int getConfig(tStream* cmdStream, char** str_bcastaddr, char** str_directaddr,
		int* flags) {
	unsigned int len;

	char* str_config = stream_rcv(cmdStream, &len);
	int* pFlags = (int*)(stream_rcv(cmdStream, &len));

	if(str_config == NULL)
		return 1;

	if(pFlags == NULL)
		return 1;

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
