#include <dhcpext/tx.h>
#include <dhcpext/rx.h>
#include <dhcpext/pc.h>
#include <dhcpext/rp.h>
#include <dhcpext/stream.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

int txSock[2], rxSock[2], pcSock[2], rpSock[2];

void sigintHandler(int);

void sigintHandler(int signo) {
	if(signo == SIGINT) {
		send(txSock[0], "shutdown", strlen("shutdown") + 1, 0);
		send(rxSock[0], "shutdown", strlen("shutdown") + 1, 0);		
	}
}

int main(int argc, char** argv) {
	if(argc != 4) {
		printf("Usage:\n\t%s <broadcast address> <direct address> <broadcast flag>\n",
			argv[0]);
		return 0;
	}

	pthread_t tx, rx, pc, rp;

	if(socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, txSock) != 0) {
		printf("Error in setting up socket pair to TX\n");
		exit(EXIT_FAILURE);
	}

	if(socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, rxSock) != 0) {
		printf("Error in setting up socket pair to RX\n");
		exit(EXIT_FAILURE);
	}

	if(socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, pcSock) != 0) {
		printf("Error in setting up socket pair to PC\n");
		exit(EXIT_FAILURE);
	}

	if(socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, rpSock) != 0) {
		printf("Error in setting up socket pair to RP\n");
		exit(EXIT_FAILURE);
	}

	if(signal(SIGINT, sigintHandler) == SIG_ERR) {
		fprintf(stderr, "Unable to catch SIGINT.\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_create(&tx, NULL, txmain, &txSock[1]) != 0) {
		fprintf(stderr, "Error: failed to start tx thread\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_create(&rx, NULL, rxmain, &rxSock[1]) != 0) {
		fprintf(stderr, "Error: failed to start rx thread\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_create(&pc, NULL, pcmain, &pcSock[1]) != 0) {
		fprintf(stderr, "Error: failed to start pc thread\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_create(&rp, NULL, rpmain, &rpSock[1]) != 0) {
		fprintf(stderr, "Error: failed to start rp thread\n");
		exit(EXIT_FAILURE);
	}

	char* str_txconfig = malloc(strlen(argv[1]) + strlen(argv[2]) + 2);
	strcpy(str_txconfig, argv[1]);
	strcat(str_txconfig, " ");
	strcat(str_txconfig, argv[2]);

	send(txSock[0], str_txconfig, strlen(str_txconfig) + 1, 0);
	free(str_txconfig);

	int flags = 0;

	if(strcmp(argv[3], "1") == 0) {
		flags |= TXFLAGS_BCAST;
	}
	
	send(txSock[0], (char*)(&flags), sizeof(int), 0);

	int tx_pc_sd[2], rx_pc_sd[2], rp_pc_sd[2];

	if(socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, tx_pc_sd) != 0) {
		printf("Error in setting up socket pair between TX and PC\n");
		exit(EXIT_FAILURE);
	}

	if(socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, rx_pc_sd) != 0) {
		printf("Error in setting up socket pair between RX and PC\n");
		exit(EXIT_FAILURE);
	}

	if(socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, rp_pc_sd) != 0) {
		printf("Error in setting up socket pair between RP and PC\n");
		exit(EXIT_FAILURE);
	}

	send(txSock[0], &tx_pc_sd[0], sizeof(int), 0);
	send(pcSock[0], &tx_pc_sd[1], sizeof(int), 0);

	usleep(100);

	send(rxSock[0], &rx_pc_sd[0], sizeof(int), 0);
	send(pcSock[0], &rx_pc_sd[1], sizeof(int), 0);

	usleep(100);

	send(rpSock[0], &rp_pc_sd[0], sizeof(int), 0);
	send(pcSock[0], &rp_pc_sd[1], sizeof(int), 0);

	pthread_join(tx, NULL);
	pthread_join(rx, NULL);
		// Wait for the threads to finish before we exit

	send(pcSock[0], "shutdown", strlen("shutdown") + 1, 0);
		// Wait for the other threads to end, before we close down
		// PC, due to the potential for data to be lost in a stream
		// without being properly free'd

	pthread_join(pc, NULL);
		// Wait for PC to finish as well

	send(rpSock[0], "shutdown", strlen("shutdown") + 1, 0);
	pthread_join(rp, NULL);
		// End RP after PC, as it will continue to send traffic through
		// until all streams have been emptied
	
	close(txSock[0]);
	close(rxSock[0]);
	close(pcSock[0]);
	close(rpSock[0]);

	return 0;
}
