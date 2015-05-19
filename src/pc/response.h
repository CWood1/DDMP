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

#ifndef __RESPONSE_H__
#define __RESPONSE_H__

#include "heartbeat.h"

#include <dhcpext/proto.h>

#include <stdint.h>
#include <netinet/in.h>

typedef struct lResponse {
	struct lResponse* next;
	struct lResponse* prev;

	response* r;
	uint8_t counter;
} lResponse;

void removeResponseFromList(lResponse**, lResponse*);
void freeResponseList(lResponse*);
void* handleUnmatchedResponse(lResponse**, response*);
int handleResponse(response*, lHeartbeat**, lResponse**, struct in_addr);
void checkUnmatchedList(lResponse**, lHeartbeat**);

#endif
