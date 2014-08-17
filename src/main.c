#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "rx.h"
#include "tx.h"
#include "pc.h"
#include "stream.h"
#include "dhcplease.h"

tStream *s_tx, *s_rx, *s_pc;

void sigintHandler(int signo) {
	if(signo == SIGINT) {
		stream_send(s_tx, "shutdown", strlen("shutdown") + 1);
		stream_send(s_rx, "shutdown", strlen("shutdown") + 1);
		stream_send(s_pc, "shutdown", strlen("shutdown") + 1);
	}
}

int main(int argc, char** argv) {
	pthread_t tx, rx, pc;

	s_tx = malloc(sizeof(tStream));
	s_rx = malloc(sizeof(tStream));
	s_pc = malloc(sizeof(tStream));

	stream_init(s_tx);
	stream_init(s_rx);
	stream_init(s_pc);

	if(signal(SIGINT, sigintHandler) == SIG_ERR) {
		fprintf(stderr, "Unable to catch SIGINT.\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_create(&tx, NULL, txmain, s_tx) != 0) {
		fprintf(stderr, "Error: failed to start tx thread\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_create(&rx, NULL, rxmain, s_rx) != 0) {
		fprintf(stderr, "Error: failed to start rx thread\n");
		exit(EXIT_FAILURE);
	}

	if(pthread_create(&pc, NULL, pcmain, s_pc) != 0) {
		fprintf(stderr, "Error: failed to start pc thread\n");
		exit(EXIT_FAILURE);
	}

	stream_send(s_tx, argv[1], strlen(argv[1]) + 1);
	stream_wait(s_tx);
	stream_send(s_tx, argv[2], strlen(argv[2]) + 1);
	stream_wait(s_tx);
	stream_send(s_tx, argv[3], strlen(argv[3]) + 1);

	tStream* s_tx_pc = malloc(sizeof(tStream));
	tStream* s_rx_pc = malloc(sizeof(tStream));

	stream_init(s_tx_pc);
	stream_init(s_rx_pc);

	stream_send(s_tx, (char*)(&s_tx_pc), sizeof(tStream*));
	stream_send(s_pc, (char*)(&s_tx_pc), sizeof(tStream*));

	stream_send(s_rx, (char*)(&s_rx_pc), sizeof(tStream*));
	stream_send(s_pc, (char*)(&s_rx_pc), sizeof(tStream*));

	printf("Hi\n");

	pthread_join(tx, NULL);
	pthread_join(rx, NULL);
	pthread_join(pc, NULL);
		// Wait for the threads to finish before we exit

	return 0;
}
