#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "stream.h"

void stream_init(tStream* stream) {
	stream->cond = malloc(sizeof(pthread_cond_t));

	if(stream->cond == NULL) {
		printf("malloc error in stream\n");
		pthread_exit(NULL);
	}

	stream->mut = malloc(sizeof(pthread_mutex_t));

	if(stream->mut == NULL) {
		printf("malloc error in stream\n");
		pthread_exit(NULL);
	}

	pthread_cond_init(stream->cond, NULL);
	pthread_mutex_init(stream->mut,  NULL);

	stream->data = NULL;
}

void stream_free(tStream* stream) {
	free(stream->cond);
	free(stream->mut);

	if(stream->data != NULL) {
		tMessage* cur = stream->data;

		while(cur->next != NULL) {
			cur = cur->next;
			free(cur->prev);
		}

		free(cur);
	}
}

void stream_send(tStream* stream, char* message, int len) {
	pthread_mutex_lock(stream->mut);

	tMessage* cur;

	if(stream->data == NULL) {
		stream->data = malloc(sizeof(tMessage));

		if(stream->data == NULL) {
			printf("malloc error in stream\n");
			pthread_exit(NULL);
		}

		cur = stream->data;
		cur->prev = NULL;
	} else {
		cur = stream->data;
		while(cur->next != NULL) {
			cur = cur->next;
		}

		cur->next = malloc(sizeof(tMessage));

		if(cur->next == NULL) {
			printf("malloc error in stream\n");
			pthread_exit(NULL);
		}

		cur->next->prev = cur;
		cur = cur->next;
	}

	cur->data = malloc(len);

	if(cur->data == NULL) {
		printf("malloc error in stream\n");
		pthread_exit(NULL);
	}

	memcpy(cur->data, message, len);
	cur->size = len;
	cur->writer = pthread_self();
	cur->next = NULL;

	pthread_mutex_unlock(stream->mut);
	pthread_cond_signal(stream->cond);
}

char* stream_rcv(tStream* stream, int* length) {
	pthread_mutex_lock(stream->mut);

	if(stream->data == NULL)
		pthread_cond_wait(stream->cond, stream->mut);

	tMessage* cur = stream->data;
	while(pthread_equal(cur->writer, pthread_self())) {
		if(cur->next) {
			cur = cur->next;
		} else {
			pthread_cond_wait(stream->cond, stream->mut);
			cur = stream->data;
				// If the stream has been read since we got
				// in here, this could be problematic
		}
	}

	(*length) = cur->size;
	char* message = malloc(cur->size);

	if(message == NULL) {
		printf("malloc error in stream\n");
		pthread_exit(NULL);
	}

	memcpy(message, cur->data, cur->size);

	if(cur->next)
		cur->next->prev = cur->prev;

	if(cur->prev)
		cur->prev->next = cur->next;

	if(cur == stream->data)
		stream->data = cur->next;

	free(cur->data);
	free(cur);

	pthread_mutex_unlock(stream->mut);
	return message;
}

char* stream_rcv_nblock(tStream* stream, int* length) {
	pthread_mutex_lock(stream->mut);

	if(stream->data == NULL) {
		pthread_mutex_unlock(stream->mut);
		return NULL;
	}

	tMessage* cur = stream->data;
	while(pthread_equal(cur->writer, pthread_self())) {
		if(cur->next) {
			cur = cur->next;
		} else {
			pthread_mutex_unlock(stream->mut);
			return NULL;
		}
	}

	(*length) = cur->size;
	char* message = malloc(cur->size);

	if(message == NULL) {
		printf("malloc error in stream\n");
		pthread_exit(NULL);
	}

	memcpy(message, cur->data, cur->size);

	if(cur->next)
		cur->next->prev = cur->prev;

	if(cur->prev)
		cur->prev->next = cur->next;

	if(cur == stream->data)
		stream->data = cur->next;

	free(cur->data);
	free(cur);

	pthread_mutex_unlock(stream->mut);
	return message;
}
