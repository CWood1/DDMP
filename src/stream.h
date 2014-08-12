#ifndef __STREAM_H__
#define __STREAM_H__

#include <pthread.h>

typedef struct {
	char* stream;
	int dataInStream;
	pthread_cond_t* cond;
	pthread_mutex_t* mut;
	pthread_t lastWrite;
} tStream;

void stream_init(tStream*);			// Initialize a stream
void stream_send(tStream*, char*, int);		// Send a string to a stream
int stream_length(tStream*);			// Length of data in stream
int stream_rcv(tStream*, int, char*);		// Receive from a stream, blocking
int stream_rcv_nblock(tStream*, int, char*);	// Receive from a stream, nonblocking
void stream_wait(tStream*);			// Wait for a stream to empty
int stream_wait_full(tStream*);			// Wait until there's some data in the stream

#endif
