CC=gcc
CFLAGS=-I. -pthread
LIBS = -lm

# Lista de archivos fuente y objetos
SRCS =  P2.c stub.c#P1.c P2.c P3.c stub.c
OBJS = $(SRCS:.c=.o)
LIBRARY = libstub.a

# Regla para generar archivos objeto
%.o: %.c
	$(CC) -c -o  $@ $< $(CFLAGS) -lm

# Regla para compilar todos los programas
all: P2 P1 P3

P1: P1.o $(LIBRARY)
	$(CC) -o $@ P1.o -L. -lstub $(LIBS)

P2: P2.o $(LIBRARY)
	$(CC) -o $@ P2.o -L. -lstub $(LIBS)

P3: P3.o $(LIBRARY)
	$(CC) -o $@ P3.o -L. -lstub $(LIBS)

# Regla para generar la biblioteca
$(LIBRARY): stub.o
	ar rcs $@ stub.o

clean:
	rm -f $(OBJS) P1 P2 P3 $(LIBRARY) P1.o P2.o P3.o