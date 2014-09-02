#ifndef __STREAM_H__
#define __STREAM_H__

#include <pthread.h>
#include <stdint.h>

typedef struct Message {
	char* data;
	size_t size;
	struct Message* next;
	struct Message* prev;
	pthread_t writer;
} tMessage;

typedef struct {
	tMessage* data;
	pthread_cond_t* cond;
	pthread_mutex_t* mut;
} tStream;

void stream_init(tStream*);				// Initialize a stream
void stream_free(tStream*);				// Clean up a stream, freeing memory
void stream_send(tStream*, const char*, size_t);	// Send a string to a stream
int stream_length(tStream*);				// Length of data in stream
char* stream_rcv(tStream*, size_t*);			// Receive from a stream, blocking
char* stream_rcv_nblock(tStream*, size_t*);		// Receive from a stream, nonblocking

#endif
