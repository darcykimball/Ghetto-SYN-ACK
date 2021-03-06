CC = clang-3.7
CFLAGS = -g -Wall -std=gnu99
LDFLAGS = ''
EXES = driver_server driver_client
TESTS = test_rit test_packet test_busywait


all: $(EXES)


tests: $(TESTS)


driver_server: driver_server.o shell.o raw_iterator.o packet.o server.o
	$(CC) -o driver_server driver_server.o shell.o raw_iterator.o packet.o \
					server.o $(LDFLAGS)


driver_client: driver_client.o shell.o raw_iterator.o packet.o client.o \
				client_commands.o busywait.o
	$(CC) -o driver_client driver_client.o shell.o raw_iterator.o packet.o \
					client.o client_commands.o busywait.o $(LDFLAGS)


test_rit: test_rit.o raw_iterator.o
	$(CC) -o test_rit test_rit.o raw_iterator.o


test_busywait: test_busywait.o busywait.o
	$(CC) -o test_busywait test_busywait.o busywait.o


test_packet: test_packet.o raw_iterator.o packet.o
	$(CC) -o test_packet test_packet.o raw_iterator.o packet.o


test_rit.o: test_rit.c
	$(CC) $(CFLAGS) -c test_rit.c


test_packet.o: test_packet.c
	$(CC) $(CFLAGS) -c test_packet.c


packet.o: packet.h packet.c
	$(CC) $(CFLAGS) -c packet.c


raw_iterator.o: raw_iterator.h raw_iterator.c
	$(CC) $(CFLAGS) -c raw_iterator.c


shell.o: shell.h shell.c
	$(CC) $(CFLAGS) -c shell.c


client_commands.o: closure.h client_commands.h client_commands.c
	$(CC) $(CFLAGS) -c client_commands.c


client.o: client.h client.c
	$(CC) $(CFLAGS) -c client.c


server.o: server.h server.c
	$(CC) $(CFLAGS) -c server.c


busywait.o: busywait.h busywait.c
	$(CC) $(CFLAGS) -c busywait.c


clean:
	rm -rf *.o $(EXES) $(TESTS)


new:
	make clean
	make all


.PHONY: all new clean tests
