dhcpext: main.o common.o stream.o proto.o tx.o rx.o pc.o rp.o dhcplease.o heartbeat.o response.o api.o
	$(CC) $(LDFLAGS) $(LIBS) main.o common.o stream.o proto.o tx.o rx.o pc.o rp.o dhcplease.o heartbeat.o response.o api.o -o dhcpext
main.o: main.c
	$(CC) $(CFLAGS) -c main.c
tx.o: tx.c
	$(CC) $(CFLAGS) -c tx.c
rx.o: rx.c
	$(CC) $(CFLAGS) -c rx.c
dhcplease.o: dhcplease.c
	$(CC) $(CFLAGS) -c dhcplease.c
stream.o: stream.c
	$(CC) $(CFLAGS) -c stream.c
proto.o: proto.c
	$(CC) $(CFLAGS) -c proto.c
pc.o: pc/pc.c
	$(CC) $(CFLAGS) -c pc/pc.c
rp.o: rp.c
	$(CC) $(CFLAGS) -c rp.c
common.o: common.c
	$(CC) $(CFLAGS) -c common.c
heartbeat.o: pc/heartbeat.c
	$(CC) $(CFLAGS) -c pc/heartbeat.c
response.o: pc/response.c
	$(CC) $(CFLAGS) -c pc/response.c
api.o: pc/api.c
	$(CC) $(CFLAGS) -c pc/api.c

clean:
	rm -r *.o dhcpext
