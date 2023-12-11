CC=gcc
CFLAGS=-I. -Wall -Wshadow -lpthread
LIBS = -lm

# Lista de archivos fuente y objetos
SRCS =  broker.c publisher.c subscriber.c stub_server.c stub_client.c
OBJS = $(SRCS:.c=.o)
LIBRARY = libstub.a

# Regla para generar archivos objeto
%.o: %.c
	$(CC) -c -o  $@ $< $(CFLAGS) -lm -g

# Regla para compilar todos los programas
all: broker publisher subscriber

broker: broker.o $(LIBRARY)
	$(CC) -o $@ broker.o -L. -lstub $(LIBS)

publisher: publisher.o $(LIBRARY)
	$(CC) -o $@ publisher.o -L. -lstub $(LIBS)

subscriber: subscriber.o $(LIBRARY)
	$(CC) -o $@ subscriber.o -L. -lstub $(LIBS)

# Regla para generar la biblioteca
$(LIBRARY): stub_server.o stub_client.o 
	ar rcs $@ stub_server.o stub_client.o 

clean:
	rm -f $(OBJS) broker publisher subscriber $(LIBRARY) broker.o publisher.o subscriber.o server_output.txt
