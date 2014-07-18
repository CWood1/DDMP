dhcpext: main.o send.o recv.o
	$(CC) $(LDFLAGS) main.o send.o recv.o -o dhcpext
main.o: main.c
	$(CC) $(CFLAGS) -c main.c
send.o: send.c
	$(CC) $(CFLAGS) -c send.c
recv.o: recv.c
	$(CC) $(CFLAGS) -c recv.c

clean:
	rm *.o dhcpext
