CC=gcc
CFLAGS=-I. -Wall -Wshadow -lpthread
LIBS = -lm

# Lista de archivos fuente y objetos
SRCS =  server.c client.c stub.c
OBJS = $(SRCS:.c=.o)
LIBRARY = libstub.a

# Regla para generar archivos objeto
%.o: %.c
	$(CC) -c -o  $@ $< $(CFLAGS) -lm -g

# Regla para compilar todos los programas
all: server client

server: server.o $(LIBRARY)
	$(CC) -o $@ server.o -L. -lstub $(LIBS)

client: client.o $(LIBRARY)
	$(CC) -o $@ client.o -L. -lstub $(LIBS)


# Regla para generar la biblioteca
$(LIBRARY): stub.o
	ar rcs $@ stub.o

clean:
	rm -f $(OBJS) server client  $(LIBRARY) server.o client.o server_output.txt
