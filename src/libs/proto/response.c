#include <dhcpext/proto.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

response* craftResponse(heartbeat* h) {
	response* r = malloc(sizeof(response));

	if(r == NULL) {
		return NULL;
	}

	r->ident = 1;
	r->flags = 0;
	r->magic = h->magic;

	return r;
}

char* serializeResponse(response* r, unsigned int* length) {
	char* s = malloc(9);

	if(s == NULL) {
		return NULL;
	}

	memset(s, 0, 9);

	memcpy(s, &(r->ident), sizeof(r->ident));
	memcpy(s + 1, &(r->flags), sizeof(r->flags));
	memcpy(s + 5, &(r->magic), sizeof(r->magic));

	*length = 9;
	return s;
}

response* deserializeResponse(char* s, unsigned int length) {
	if(length != 9) {
		errno = EINVAL;
		return NULL;
	}

	response* r = malloc(sizeof(response));

	if(r == NULL) {
		return NULL;
	}

	memcpy(&(r->ident), s, sizeof(r->ident));
	memcpy(&(r->flags), s + 1, sizeof(r->flags));
	memcpy(&(r->magic), s + 5, sizeof(r->magic));

	return r;
}

void printResponse(response* r) {
	printf("\tType:\t%s\n\tFlags:\t%x\n\tMagic:\t%x\n",
		((r->ident == 1) ? "Response" : ""),
		r->flags, r->magic);
}

void freeResponse(response* r) {
	if(r == NULL) {
		return;
	}

	free(r);
}
