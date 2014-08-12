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
	memcpy(stream->stream, message, len);
	stream->dataInStream = len;

	stream->lastWrite = pthread_self();

	pthread_mutex_unlock(stream->mut);
	pthread_cond_signal(stream->cond);
}

int stream_length(tStream* stream) {
	return stream->dataInStream;
}

int stream_rcv(tStream* stream, int length, char* dest) {
	pthread_mutex_lock(stream->mut);

	if(stream->dataInStream == 0)
		pthread_cond_wait(stream->cond, stream->mut);
	if(pthread_equal(stream->lastWrite, pthread_self()))
		pthread_cond_wait(stream->cond, stream->mut);

	if(length == 0) {
		length = stream->dataInStream;
		memcpy(dest, stream->stream, stream->dataInStream);
		stream->dataInStream = 0;
	} else {
		memcpy(dest, stream->stream, length);

		if(length != stream->dataInStream) {
			stream->dataInStream -= length;

			char* message = malloc(stream->dataInStream);
			memcpy(message, stream->stream + length, stream->dataInStream);

			free(stream->stream);
			stream->stream = message;
		} else {
			stream->dataInStream = 0;
			free(stream->stream);
		}
	}

	pthread_mutex_unlock(stream->mut);
	return length;
}

int stream_rcv_nblock(tStream* stream, int length, char* dest) {
        pthread_mutex_lock(stream->mut);

        if(stream->dataInStream == 0) {
                pthread_mutex_unlock(stream->mut);
		return 0;
	}

        if(pthread_equal(stream->lastWrite, pthread_self())) {
		pthread_mutex_unlock(stream->mut);
		return 0;
	}

        if(length == 0) {
                length = stream->dataInStream;
                memcpy(dest, stream->stream, stream->dataInStream);
                stream->dataInStream = 0;
        } else {
                memcpy(dest, stream->stream, length);

                if(length != stream->dataInStream) {
                        stream->dataInStream -= length;

                        char* message = malloc(stream->dataInStream);
                        memcpy(message, stream->stream + length, stream->dataInStream);

                        free(stream->stream);
                        stream->stream = message;
                } else {
                        stream->dataInStream = 0;
                        free(stream->stream);
                }
        }

        pthread_mutex_unlock(stream->mut);
        return length;
}

void stream_wait(tStream* stream) {
	char Done = 0;

	while(Done == 0) {
		pthread_mutex_lock(stream->mut);
		if(stream->dataInStream == 0) Done = 1;
		pthread_mutex_unlock(stream->mut);
	}
}

int stream_wait_full(tStream* stream) {
	char d = 0;

	while(d == 0) {
		pthread_mutex_lock(stream->mut);
		if(stream->dataInStream != 0) d = 1;
		pthread_mutex_unlock(stream->mut);
	}

	return stream->dataInStream;
}
