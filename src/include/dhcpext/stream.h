/**
    This file is part of mesh-dhcp-ext, the Mesh Network DHCP Extensions protocol.

    mesh-dhcp-ext is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    mesh-dhcp-ext is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with mesh-dhcp-ext.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2015 Connor Wood <connorwood71@gmail.com>.
*/

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
