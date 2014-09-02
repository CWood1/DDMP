#include <dhcpext/proto.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

heartbeat* craftHeartbeat(int active) {
	heartbeat* h = malloc(sizeof(heartbeat));

	if(h == NULL) {
		printf("malloc error in proto\n");
		pthread_exit(NULL);
	}

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
	char* s = malloc(9);

	if(s == NULL) {
		printf("malloc error in proto\n");
		pthread_exit(NULL);
	}

	memset(s, 0, 9);

	s[0] = h->ident;
	memcpy(s + 1, &(h->flags), sizeof(h->flags));
	memcpy(s + 5, &(h->magic), sizeof(h->magic));

	*length = 9;
	return s;
}

heartbeat* deserializeHeartbeat(char* s, int length) {
	if(length != 9) {
		return NULL;
	}

	heartbeat* h = malloc(sizeof(heartbeat));

	if(h == NULL) {
		printf("malloc error in proto\n");
		pthread_exit(NULL);
	}

	h->ident = s[0];
	memcpy(&(h->flags), s + 1, sizeof(h->flags));
	memcpy(&(h->magic), s + 5, sizeof(h->magic));

	h->l = NULL;
	h->n = NULL;

	return h;
}

void printHeartbeat(heartbeat* h) {
	printf("\tType:\t%s\n\tFlags:\t%x\n\tMagic:\t%x\n", 
		((h->ident == 0) ? "Heartbeat" : ""),
		h->flags, h->magic);
}

void freeHeartbeat(heartbeat* h) {
	if(h->l != NULL) {
		free(h->l);
	}

	if(h->n != NULL) {
		free(h->n);
	}
		// Both of these are crude, and do not free a linked list.
		// When UCI integration happens, these will need rewriting.

	free(h);
}
