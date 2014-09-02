#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "dhcplease.h"

#define NETWORK "10.0.0.0"
#define SUBNET "255.0.0.0"
	// Very temporary

int checkSubnet(const char* ipAddress, const char* desiredNetwork, const char* subnet) {
	in_addr_t ip, net, mask;

	ip = inet_addr(ipAddress);
	net = inet_addr(desiredNetwork);
	mask = inet_addr(subnet);

	if((ip & mask) == net) {
		return 1;
	} else {
		return 0;
	}
}

void initDhcpLease() {
	unsigned long size = 0;
	char* buffer = NULL;
	FILE* leasefile = fopen("/tmp/dhcp.leases", "r");
	stringList* l = malloc(sizeof(stringList));
	dhcpEntry* leases = malloc(sizeof(dhcpEntry));

	l->prev = NULL;
	leases->prev = NULL;

	if(leasefile == NULL) {
		fputs("Unable to open lease file.\n", stderr);
		exit(1);
	}

	fseek(leasefile, 0, SEEK_END);
	size = ftell(leasefile);
	rewind(leasefile);

	buffer = malloc(sizeof(char) * size);
	if(buffer == NULL) {
		fputs("Unable to allocate buffer for file.\n", stderr);
		exit(1);
	}

	if(fread(buffer, 1, size, leasefile) != size) {
		fputs("Error reading lease file.\n", stderr);
		exit(1);
	}

	l->contents = strtok(buffer, "\n");
	while(l->contents != NULL) {
		l->next = malloc(sizeof(stringList));
		l->next->prev = l;
		l->next->next = NULL;
		l = l->next;

		l->contents = strtok(NULL, "\n");
	}
		// Done because mixing calls to strtok is not allowed, and we need to

	while(l->prev != NULL) {
		l = l->prev;
	}

	while(l->contents != NULL) {
		int i = 0;
		char* contents = strtok(l->contents, " ");

		while(contents != NULL) {
			switch(i++) {
				case 0:
					leases->time = contents;
					break;
				case 1:
					leases->macaddr = contents;
					break;
				case 2:
					leases->allocatedip = contents;
					break;
				case 3:
					leases->hostname = contents;
					break;
				case 4:
					leases->theotherfield = contents;
					break;
			}

			contents = strtok(NULL, " ");
		}

		leases->next = malloc(sizeof(dhcpEntry));
		leases->next->next = NULL;
		leases->next->prev = leases;
		leases = leases->next;

		l = l->next;
	}

	while(leases->prev != NULL) {
		leases = leases->prev;
	}

	printf("Time\tMAC Address\tIP Address\tHostname\tSomething\n\n");

	while(leases->time != NULL) {
		if(checkSubnet(leases->allocatedip, NETWORK, SUBNET) == 1) {
			printf("%s\t%s\t%s\t%s\t%s\n", leases->time, leases->macaddr,
				leases->allocatedip, leases->hostname, leases->theotherfield);
			leases = leases->next;
		} else {
			dhcpEntry* cur = leases;

			leases->prev->next = leases->next;
			leases->next->prev = leases->prev;
			leases = leases->next;
			free(cur);
		}
	}
}

