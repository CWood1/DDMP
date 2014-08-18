#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "stream.h"

void stream_init(tStream* stream) {
	stream->cond = malloc(sizeof(pthread_cond_t));
	stream->mut = malloc(sizeof(pthread_mutex_t));

	pthread_cond_init(stream->cond, NULL);
	pthread_mutex_init(stream->mut,  NULL);

	stream->data = NULL;
}

void stream_send(tStream* stream, char* message, int len) {
	pthread_mutex_lock(stream->mut);

	if(stream->data == NULL) {
		stream->data = malloc(sizeof(tMessage));
		stream->data->data = malloc(len);
		memcpy(stream->data->data, message, len);
		stream->data->next = NULL;
		stream->data->prev = NULL;
		stream->data->size = len;
		stream->data->writer = pthread_self();
	} else {
		tMessage* cur = stream->data;

		while(cur->next != NULL) {
			cur = cur->next;
		}

		cur->next = malloc(sizeof(tMessage));
		cur->next->prev = cur;
		cur = cur->next;

		cur->data = malloc(len);
		memcpy(cur->data, message, len);
		cur->size = len;
		cur->writer = pthread_self();
		cur->next = NULL;
	}

	pthread_mutex_unlock(stream->mut);
	pthread_cond_signal(stream->cond);
}

int stream_length(tStream* stream) {
	if(stream->data != NULL) {
		return stream->data->size;
	} else {
		return 0;
	}
}

int stream_rcv(tStream* stream, int length, char* dest) {
	pthread_mutex_lock(stream->mut);

	if(stream->data == NULL)
		pthread_cond_wait(stream->cond, stream->mut);

	tMessage* cur = stream->data;
	while(pthread_equal(cur->writer, pthread_self())) {
		if(cur->next) {
			cur = cur->next;
		} else {
			pthread_cond_wait(stream->cond, stream->mut);
		}
	}

	if(length == 0) {
		length = cur->size;
		memcpy(dest, cur->data, cur->size);

		if(cur->next)
			cur->next->prev = cur->prev;

		if(cur->prev)
			cur->prev->next = cur->next;

		if(cur == stream->data)
			stream->data = cur->next;

		free(cur->data);
		free(cur);
	} else {
		memcpy(dest, cur->data, length);

		if(length != cur->size) {
			cur->size -= length;

			char* message = malloc(cur->size);
			memcpy(message, cur->data + length, cur->size);

			free(cur->data);
			cur->data = message;
		} else {
			if(cur->next)
				cur->next->prev = cur->prev;

			if(cur->prev)
				cur->prev->next = cur->next;

			if(cur == stream->data)
				stream->data = cur->next;

			free(cur->data);
			free(cur);
		}
	}

	pthread_mutex_unlock(stream->mut);
	return length;
}

int stream_rcv_nblock(tStream* stream, int length, char* dest) {
	pthread_mutex_lock(stream->mut);

	if(stream->data == NULL) {
		pthread_mutex_unlock(stream->mut);
		return 0;
	}

	tMessage* cur = stream->data;
	while(pthread_equal(cur->writer, pthread_self())) {
		if(cur->next) {
			cur = cur->next;
		} else {
			pthread_mutex_unlock(stream->mut);
			return 0;
		}
	}

	if(length == 0) {
		length = cur->size;
		memcpy(dest, cur->data, cur->size);

		if(cur->next)
			cur->next->prev = cur->prev;

		if(cur->prev)
			cur->prev->next = cur->next;

		if(cur == stream->data)
			stream->data = cur->next;

		free(cur->data);
		free(cur);
	} else {
		memcpy(dest, cur->data, length);

		if(length != cur->size) {
			cur->size -= length;

			char* message = malloc(cur->size);
			memcpy(message, cur->data + length, cur->size);

			free(cur->data);
			cur->data = message;
		} else {
			if(cur->next)
				cur->next->prev = cur->prev;

			if(cur->prev)
				cur->prev->next = cur->next;

			if(stream->data == cur)
				stream->data = cur->next;

			free(cur->data);
			free(cur);
		}
	}

	pthread_mutex_unlock(stream->mut);
	return length;
}

void stream_wait(tStream* stream) {
	uint8_t d = 0;

	while(d == 0) {
		pthread_mutex_lock(stream->mut);
		if(stream->data == NULL) d = 1;
		pthread_mutex_unlock(stream->mut);
	}
}

int stream_wait_full(tStream* stream) {
	pthread_mutex_lock(stream->mut);
	if(stream->data == NULL)
		pthread_cond_wait(stream->cond, stream->mut);
	pthread_mutex_unlock(stream->mut);

	return stream->data->size;
}
