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

typedef struct {
    int client_socket;
} ThreadArgs;


void *
handle_client(void *arg)
{
	ThreadArgs *threadArgs = (ThreadArgs *)arg;
    int client_socket = threadArgs->client_socket;

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

	sleep(numeroAleatorio);
	if (send(client_socket, path, strlen(path), 0) < 0) {
		perror("send");
	}

	close(client_socket);
	free(threadArgs);
	free(path);
	return NULL;
}

int
main(int argc, char *argv[])
{
	char *endptr;
	int total_threads = 0;
    long port = strtol(argv[1], &endptr, 10);
	

    if (*endptr != '\0' && *endptr != '\n') {
        printf("No se pudo convertir a entero.\n");
        return 1;
    }

	setbuf(stdout, NULL);

	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//Meter ips si no no funciona desde otra maquina 
	server_addr.sin_port = htons(port);


	int addrlen = sizeof(server_addr);


	signal(SIGINT, handler);

	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (tcp_socket < 0) {
		perror("socket");
		close(tcp_socket);
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
		close(tcp_socket);
		perror("bind failure");
		exit(EXIT_FAILURE);
	}
	printf("Socket successfully binded...\n");

	if (listen(tcp_socket, 1024) < 0) {
		perror("listen");
		close(tcp_socket);
		exit(EXIT_FAILURE);
	}
	printf("Server listening…\n");

	pthread_t threads[1024];
	
	while (!exit_flag) {
		int *client_socket;
		
		if (num_threads < 100) {
			client_socket = malloc(sizeof(int));
			
		
			if ((*client_socket =
		     	accept(tcp_socket, (struct sockaddr *)&server_addr, (socklen_t *) & addrlen)) < 0) {
				perror("accept");
				close(*client_socket);
				close(tcp_socket);
				exit(EXIT_FAILURE);//esto no se si esta bien
			}
			
			
			ThreadArgs *threadArgs = (ThreadArgs *)malloc(sizeof(ThreadArgs));
		
			
			
			threadArgs->client_socket = *client_socket;
			if (pthread_create(&threads[num_threads], NULL, handle_client, (void*) threadArgs) != 0) {
				perror("Error al crear el hilo");
				free(client_socket);
				free(threadArgs);
			} else {
				num_threads++;
				total_threads++;
			}
		} else {
			fprintf(stderr,
				"Max clients reached. Rejecting new connection.\n");
			for (int i = 0; i < 100; i++) {
				printf("Waiting for thread %d...\n", i);
				if (pthread_join(threads[i], NULL) != 0) {
					perror("Error al esperar el hilo");
					continue;
				}
			
			}
			
			num_threads = 0;
			
			continue;
		}
		
		//close(*client_socket);
		free(client_socket);
	}
	printf("Waiting for threads to finish...\n");
	
	for (int i = 0; i < num_threads; i++) {
		//printf("Waiting for thread %d...\n", i);
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
