CC=gcc
CFLAGS=-I.
DEPS = 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: server.o
	gcc -o server server.o	
client: client.o
	gcc -o client client.o
	
clean:
	rm -f *.o server client
