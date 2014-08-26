dhcpext: main.o stream.o proto.o tx.o rx.o pc.o rp.o dhcplease.o
	$(CC) $(LDFLAGS) $(LIBS) main.o stream.o proto.o tx.o rx.o pc.o rp.o dhcplease.o -o dhcpext
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
pc.o: pc.c
	$(CC) $(CFLAGS) -c pc.c
rp.o: rp.c
	$(CC) $(CFLAGS) -c rp.c

clean:
	rm *.o dhcpext
