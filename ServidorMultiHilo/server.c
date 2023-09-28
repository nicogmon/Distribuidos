#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>		/* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>



int exit_flag = 0;
int num_threads = 0;

void
handler(int number)
{
	switch (number) {
	case SIGINT:
		fprintf(stderr, "SIGINT received!\n");
		exit_flag = 1;
		break;
	
	case SIGPIPE:
		fprintf(stderr, "SIGPIPE received!\n");
		num_threads--;
		break;
	}
}

void *
handle_client(void *arg)
{

	int client_socket = *(int *)arg;
	char buffer[1024] = { 0 };
	char *path = (char *)malloc(sizeof(char) * 1024);

	strcpy(path, "Hello client");

	memset(buffer, 0, sizeof(buffer));
	if (recv(client_socket, buffer, 1024, 0) > 0) {	//comprobar si el error es diferente de nd que leer
		printf(">%s\n", buffer);
	}

	srand(time(NULL));

    // Genera un número aleatorio entre 0.0 y 1.0
    double randomDouble = (double)rand() / RAND_MAX;

    // Ajusta el número al rango entre 0.5 y 2.0
    double numeroAleatorio = 0.5 + randomDouble * (2.0 - 0.5);

	sleep(10);
	if (send(client_socket, path, strlen(path), 0) < 0) {
		perror("send");
	}

	signal(SIGINT, handler);
	if (exit_flag == 1) {
		
		close(client_socket);
		return NULL;
	}
	num_threads--;
	free(arg);
	return NULL;
}

int
main(int argc, char *argv[])
{
	char *endptr;
	int * p = malloc(sizeof(int));
  long port = strtol(argv[1], &endptr, 10);

    if (*endptr != '\0' && *endptr != '\n') {
        printf("No se pudo convertir a entero.\n");
        return 1;
    }

    printf("El valor entero es: %ld\n", port);

	setbuf(stdout, NULL);

	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//Meter ips si no no funciona desde otra maquina 
	server_addr.sin_port = htons(port);


	int addrlen = sizeof(server_addr);

	char buffer[1024] = { 0 };
	char *hello = "Hello client";
	char *path = (char *)malloc(sizeof(char) * 1024);

	signal(SIGINT, handler);

	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (tcp_socket < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	printf("Socket successfully created...\n");
	const int enable = 1;

	if (setsockopt
	    (tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");

	if (bind
	    (tcp_socket, (struct sockaddr *)&server_addr,
	     sizeof(server_addr)) < 0) {
		perror("bind failure");
		exit(EXIT_FAILURE);
	}
	printf("Socket successfully binded...\n");

	if (listen(tcp_socket, 1024) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("Server listening…\n");

	pthread_t threads[1024];
	
	while (!exit_flag) {
		int *client_socket = malloc(sizeof(int));

		printf("Waiting for new connection...\n");
		if (num_threads == 100){
			printf("Max clients reached. Waiting for a thread to finish...\n");
			continue;
		}
		if ((*client_socket =
		     accept(tcp_socket, (struct sockaddr *)&server_addr,
			    (socklen_t *) & addrlen)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);//esto no se si esta bien
		}
		printf("New connection accepted...\n");

		if (num_threads < 100) {
			if (pthread_create
			    (&threads[num_threads], NULL, handle_client,
			     client_socket) != 0) {
				perror("Error al crear el hilo");
				free(client_socket);
			} else {
				num_threads++;
			}
		} else {
			fprintf(stderr,
				"Max clients reached. Rejecting new connection.\n");
			close(*client_socket);
			free(client_socket);
		}

	}
	printf("Waiting for threads to finish...\n");
	for (int i = 0; i < num_threads; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			perror("Error al esperar el hilo");
			return -1;
		}
	}
	close(tcp_socket);
	printf("Exiting...\n");
	exit(EXIT_SUCCESS);
	return 0;
}
