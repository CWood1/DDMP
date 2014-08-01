#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stream.h"

void stream_init(tStream* stream) {
	stream->cond = malloc(sizeof(pthread_cond_t));
	stream->mut = malloc(sizeof(pthread_mutex_t));

	pthread_cond_init(stream->cond, NULL);
	pthread_mutex_init(stream->mut,  NULL);

	stream->dataInStream = 0;
}

void stream_send(tStream* stream, char* message, int len) {
	pthread_mutex_lock(stream->mut);

	stream->stream = malloc(len);
	strcpy(stream->stream, message);
	stream->dataInStream = 1;

	stream->lastWrite = pthread_self();

	pthread_mutex_unlock(stream->mut);
	pthread_cond_signal(stream->cond);
}

char* stream_rcv(tStream* stream) {
	pthread_mutex_lock(stream->mut);
	if(stream->dataInStream == 0) pthread_cond_wait(stream->cond, stream->mut);
	if(pthread_equal(stream->lastWrite, pthread_self())) pthread_cond_wait(stream->cond, stream->mut);

	char* message;
	message = malloc(strlen(stream->stream) + 1);
	strcpy(message, stream->stream);

	stream->dataInStream = 0;
	free(stream->stream);

	pthread_mutex_unlock(stream->mut);
	return message;
}

char* stream_rcv_nblock(tStream* stream) {
	pthread_mutex_lock(stream->mut);
	if(stream->dataInStream == 0) {
	       pthread_mutex_unlock(stream->mut);
       	       return NULL;
	}

	if(pthread_equal(stream->lastWrite, pthread_self())) {
	       pthread_mutex_unlock(stream->mut);
       	       return NULL;
	}

	char* message;
	message = malloc(strlen(stream->stream) + 1);
	strcpy(message, stream->stream);

	stream->dataInStream = 0;
	free(stream->stream);

	pthread_mutex_unlock(stream->mut);
	return message;
}

void stream_wait(tStream* stream) {
	char Done = 0;

	while(Done == 0) {
		pthread_mutex_lock(stream->mut);
		if(stream->dataInStream == 0) Done = 1;
		pthread_mutex_unlock(stream->mut);
	}
}
