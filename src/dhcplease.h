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
