CC=gcc
CFLAGS=-I -Wall -Wshadow -lpthread.
DEPS = 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: server client

server: server.c 
	gcc -g -Wall -o server server.c

client: client.c 
	gcc -g -Wall -o client client.c
	
clean:
	rm -f *.o server client
