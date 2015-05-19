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

#ifndef __DHCPLEASE_H__
#define __DHCPLEASE_H__

typedef struct stringList {
	char* contents;
	struct stringList* next;
	struct stringList* prev;
} stringList;

typedef struct dhcpEntry {
	char* time;
	char* macaddr;
	char* allocatedip;
	char* hostname;
	char* theotherfield;
		// TODO: Find out WTF this does

	struct dhcpEntry* next;
	struct dhcpEntry* prev;
} dhcpEntry;

int checkSubnet(const char*, const char*, const char*);
void initDhcpLease(void);

#endif
