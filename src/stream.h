#ifndef __STREAM_H__
#define __STREAM_H__

#include <pthread.h>

typedef struct {
	char* stream;
	char dataInStream;
	pthread_cond_t* cond;
	pthread_mutex_t* mut;
	pthread_t lastWrite;
} tStream;

void stream_init(tStream*);		// Initialize a stream
void stream_send(tStream*, char*, int);	// Send a string to a stream
char* stream_rcv(tStream*);		// Receive from a stream, blocking
char* stream_rcv_nblock(tStream*);	// Receive from a stream, nonblocking
void stream_wait(tStream*);		// Wait for a stream to empty

#endif
