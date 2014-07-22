dhcpext: main.o send.o recv.o dhcplease.o
	$(CC) $(LDFLAGS) main.o send.o recv.o dhcplease.o -o dhcpext
main.o: main.c
	$(CC) $(CFLAGS) -c main.c
send.o: send.c
	$(CC) $(CFLAGS) -c send.c
recv.o: recv.c
	$(CC) $(CFLAGS) -c recv.c
dhcplease.o: dhcplease.c
	$(CC) $(CFLAGS) -c dhcplease.c

clean:
	rm *.o dhcpext
