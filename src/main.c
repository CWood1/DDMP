#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "rx.h"
#include "tx.h"
#include "stream.h"
#include "dhcplease.h"

tStream *s_tx, *s_rx;

void sigintHandler(int signo) {
	if(signo == SIGINT) {
		stream_send(s_tx, "shutdown", strlen("shutdown") + 1);
		stream_send(s_rx, "shutdown", strlen("shutdown") + 1);
	}
}

int main(int argc, char** argv) {
	pthread_t tx, rx;

	s_tx = malloc(sizeof(tStream));
	s_rx = malloc(sizeof(tStream));

	stream_init(s_tx);
	stream_init(s_rx);

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

	stream_send(s_tx, argv[1], strlen(argv[1]) + 1);
	stream_wait(s_tx);
	stream_send(s_tx, argv[2], strlen(argv[2]) + 1);
	stream_wait(s_tx);
	stream_send(s_tx, argv[3], strlen(argv[3]) + 1);

	pthread_join(tx, NULL);
	pthread_join(rx, NULL);
		// Wait for the threads to finish before we exit

	return 0;
}
