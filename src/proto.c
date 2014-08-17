#include "proto.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

heartbeat* craftHeartbeat(int active) {
	heartbeat* h = malloc(sizeof(heartbeat));

	h->ident = 0;
	h->flags = 0;

	if(active == 1) {
		h->flags |= FLAG_ACTIVE;
	}
	
	int random = open("/dev/urandom", O_RDONLY);
	read(random, &(h->magic), sizeof(h->magic));
	close(random);
		// Generate a random number for the magic number, used to
		// check lack of errors on the line

	h->l = NULL;
	h->n = NULL;

	return h;
}

char* serializeHeartbeat(heartbeat* h, int* length) {
	char* heartbeat = malloc(9);
	memset(heartbeat, 0, 9);

	heartbeat[0] = h->ident;
	memcpy(heartbeat + 1, &(h->flags), sizeof(h->flags));
	memcpy(heartbeat + 5, &(h->magic), sizeof(h->magic));

	*length = 9;
	return heartbeat;
}

heartbeat* deserializeHeartbeat(char* s, int length) {
	if(length != 9) {
		return NULL;
	}

	heartbeat* h = malloc(sizeof(heartbeat));

	h->ident = s[0];
	memcpy(&(h->flags), s + 1, sizeof(h->flags));
	memcpy(&(h->magic), s + 5, sizeof(h->magic));

	return h;
}

void printHeartbeat(heartbeat* h) {
	printf("\tType:\t%s\n\tFlags:\t%x\n\tMagic:\t%x\n", 
		((h->ident == 0) ? "Heartbeat" : ""),
		h->flags, h->magic);
}

int isHeartbeat(char* c, int length) {
	if(length == 9 && c[0] == 0) {
		return 1;
	} else if(length == 9 && c[0] == 1) {
		return 0;
	} else {
		return -1;
	}
}

response* craftResponse(heartbeat* h) {
	response* r = malloc(sizeof(response));

	r->ident = 1;
	r->flags = 0;
	r->magic = h->magic;

	return r;
}

char* serializeResponse(response* r, int* length) {
	char* response = malloc(9);
	memset(response, 0, 9);

	response[0] = r->ident;
	memcpy(response + 1, &(r->flags), sizeof(r->flags));
	memcpy(response + 5, &(r->magic), sizeof(r->magic));

	*length = 9;
	return response;
}

response* deserializeResponse(char* s, int length) {
	if(length != 9) {
		return NULL;
	}

	response* r = malloc(sizeof(response));

	r->ident = s[0];
	memcpy(&(r->flags), s + 1, sizeof(r->flags));
	memcpy(&(r->magic), s + 5, sizeof(r->magic));

	return r;
}

void printResponse(response* r) {
	printf("\tType:\t%s\n\tFlags:\t%x\n\tMagic:\t%x\n",
		((r->ident == 1) ? "Response" : ""),
		r->flags, r->magic);
}
