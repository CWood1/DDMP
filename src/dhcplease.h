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

unsigned int addrStringToInt(char*);
int checkSubnet(char*, char*, char*);
void initDhcpLease(void);

#endif
