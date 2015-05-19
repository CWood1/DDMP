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

#ifndef __PROTO_H__
#define __PROTO_H__

#include <stdint.h>
#include <string.h>

typedef struct {
	uint32_t filelen;
	char* file;
} leasefile;

typedef struct nodelist {
	char* ip;
	struct nodelist* next;
} nodelist;

typedef struct {
	uint8_t ident;
	uint32_t flags;
	uint32_t magic;

	leasefile* l;
	nodelist* n;
} heartbeat;

typedef struct {
	uint8_t ident;
	uint32_t flags;
	uint32_t magic;

	uint32_t numnodes;
	uint8_t shutdown;
		// Only ever used if D flag is set
} response;

#define FLAG_ACTIVE 1
#define FLAG_SHUTDOWN 2

#define TYPE_HEARTBEAT 1
#define TYPE_RESPONSE 0

heartbeat* craftHeartbeat(int);
char* serializeHeartbeat(heartbeat*, unsigned int*);
heartbeat* deserializeHeartbeat(char*, size_t);
void printHeartbeat(heartbeat*);
void freeHeartbeat(heartbeat*);

response* craftResponse(heartbeat*);
char* serializeResponse(response*, unsigned int*);
response* deserializeResponse(char*, size_t);
void printResponse(response*);
void freeResponse(response*);

int isHeartbeat(char*, size_t);

#endif
