CC = clang-3.7
CFLAGS = -g -Wall -std=c99
LDFLAGS = ''
EXES = driver_server driver_client


all: $(EXES)


driver_server: driver_server.o shell.o raw_iterator.o packet.o \
				client_commands.o server.o
	$(CC) -o driver_server driver_server.o shell.o raw_iterator.o packet.o client_commands.o


driver_client: driver_client.o shell.o raw_iterator.o packet.o server.o
	$(CC) -o driver_client driver_client.o shell.o raw_iterator.o packet.o server.o


packet.o: packet.h packet.c
	$(CC) $(CFLAGS) -c packet.c


raw_iterator.o: raw_iterator.h raw_iterator.c
	$(CC) $(CFLAGS) -c raw_iterator.c


shell.o: shell.h shell.c
	$(CC) $(CFLAGS) -c shell.c


client_commands.o: client_commands.h client_commands.c
	$(CC) $(CFLAGS) -c client_commands.c


client.o: client.h client.c
	$(CC) $(CFLAGS) -c client.c


server.o: server.h server.c
	$(CC) $(CFLAGS) -c server.c


clean:
	rm -rf *.o $(EXES)


new:
	make clean
	make all


.PHONY: all new clean
