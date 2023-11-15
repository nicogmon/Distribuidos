#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>	
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include <semaphore.h>
#include "stub.h"

#define MAX_CLIENTS 600
#define PID_LENGTH 5
#define MAX_CONNECTIONS 1000
#define FALSE 0
#define TRUE 1


unsigned int clock_lamport = 0;

int tcp_socket;

int tcp_sockets [MAX_CONNECTIONS];
int client_counter = 0;
char Pid [PID_LENGTH];

pthread_mutex_t mutex;
sem_t semaphore;


pthread_t threads_not_collected [MAX_CLIENTS];
int nthreads_not_collected = 0;

int counter;

int init_Server(long port) {
    
    setbuf(stdout, NULL);
    
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

    int addrlen = sizeof(server_addr);
    
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    const int enable = 1;
    
    if (setsockopt
	    (tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
		perror("setsockopt(SO_REUSEADDR) failed");
    } 

    if (bind
         (tcp_socket, (struct sockaddr *)&server_addr,
	     sizeof(server_addr)) < 0) {
		perror("bind failure");
		exit(EXIT_FAILURE);
	}
    
    if (listen(tcp_socket, MAX_CONNECTIONS) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
    
    pthread_t thread_id;
    threadArgs  * args = (threadArgs *) malloc(sizeof(threadArgs));
    args->server_addr = server_addr;
    args->addrlen = addrlen;

    //creamos un hlo que se encarge de aceptar las conexiones para poder volver al programa principal
    pthread_create(&thread_id, NULL, accept_connections, (void *) args);
    threads_not_collected[nthreads_not_collected] = thread_id;
    nthreads_not_collected++;

    
    FILE * output_file = fopen("server_output.txt", "w+");
    if (output_file == NULL) {
        perror("Error opening server_output file");
        exit(EXIT_FAILURE);
    }
    
    fprintf(output_file, "%d", 0);
    fclose(output_file);
    return 0;
    }

// establish connections between processes
void * accept_connections(void * args) {
    pthread_t thread_ids[MAX_CLIENTS];
    threadArgs * arguments =(threadArgs *) args;
    struct sockaddr_in * server_addr = &arguments->server_addr;
    int * addrlen = &arguments->addrlen;

    if (sem_init(&semaphore, 0, MAX_CLIENTS) == -1) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_CONNECTIONS; i++){
        int local_socket;
        
        if ((local_socket = accept(tcp_socket, (struct sockaddr *)server_addr,
         (socklen_t *)addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        
        void * recv_args = malloc(sizeof(int));
        *(int *) recv_args = local_socket;
        
        //creamos un hilo por cada conexion que se quedar escchando indefinidadmente
        //sin interrumpir la ejecucion del hilo principal 
        pthread_create(&thread_ids[i], NULL, server_receive, recv_args);
    }

    

    for (int i = 0; i < MAX_CLIENTS; i++){
        pthread_join(thread_ids[i], NULL);
    }
    
    return 0;  
}



void * server_receive(void *arg) {   
    
    //int socket_local = tcp_sockets[client_counter];
    
    int socket_local = *(int *) arg;
    struct timespec clock_time , start_time , current_time;
    
    request * msg = malloc(sizeof(request));
    
    sem_wait(&semaphore);
    
    

    if (recv(socket_local, msg, sizeof(request), 0) <= 0) {	
        perror("recv");
    }
    
    srand(time(0));
    int rand_sleep_ms = rand() % (150 - 75 + 1) + 75;
    
    
   
    clock_gettime(CLOCK_REALTIME, &clock_time);
    double time_stamp = (clock_time.tv_sec + (clock_time.tv_nsec / 1e9)) ;
    
    
    
    if (msg -> action == WRITE) {
        clock_gettime(CLOCK_REALTIME, &start_time);
        pthread_mutex_lock(&mutex);
        clock_gettime(CLOCK_REALTIME, &current_time);
        FILE * output_file = fopen("server_output.txt", "r+");
        
        fscanf(output_file, "%d", &counter);
        
        
        //leer valor
        
        counter++;
        if (fseek(output_file, 0, SEEK_SET) < 0) {
            perror("fseek");
            sem_post(&semaphore);
            pthread_mutex_unlock(&mutex);
            close (socket_local);
            return NULL;
        }
        if (fprintf(output_file, "%d", counter) < 0) {
            perror("write");
            sem_post(&semaphore);
            pthread_mutex_unlock(&mutex);
            close (socket_local);
            return NULL;
        }
        
        printf("[%f][ESCRITOR %d] modifica contador con valor %d\n",time_stamp , msg->id, counter);
        usleep(rand_sleep_ms * 1000);
        //printf("%d\n", rand_sleep_ms * 1000);
        pthread_mutex_unlock(&mutex);
        fclose(output_file);
        
    }
    else if (msg -> action == READ) {
        FILE * output_file = fopen("server_output.txt", "r");
        clock_gettime(CLOCK_REALTIME, &start_time);
        pthread_mutex_lock(&mutex);
        clock_gettime(CLOCK_REALTIME, &current_time);
        pthread_mutex_unlock(&mutex);
        
        printf("[%f][LECTOR %d] lee contador con valor %d\n",time_stamp , msg->id, counter);
        //printf("%d\n", rand_sleep_ms * 1000);
        usleep(rand_sleep_ms * 1000);
        fclose(output_file);
        
    }
    double elapsed = (current_time.tv_sec  + (current_time.tv_nsec / 1e9)) - (start_time.tv_sec +  (start_time.tv_nsec / 1e9));
    //printf("Elapsed time: %f\n", elapsed);
    response res;
    res.action = msg->action;
    res.counter = counter;
    res.latency_time = elapsed * 1e9;
    if (send(socket_local, &res, sizeof(response), 0) < 0) {
        perror("send");
        sem_post(&semaphore);
        close (socket_local);
        return NULL;
    }
    sem_post(&semaphore);
    //free(msg);
    
    
    
   
}

int init_Client(char * ip, long port, int id) {
    
    if ((tcp_sockets[id] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct in_addr ipv4Addr;

    if (inet_pton(AF_INET, ip, &ipv4Addr) <= 0) {
        perror("inet_pton() failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = ipv4Addr.s_addr;	
    server_addr.sin_port = htons(port);

    int addrlen = sizeof(server_addr);

    if (connect(tcp_sockets[id], (struct sockaddr *) &server_addr, addrlen) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    return tcp_sockets[id];
}

void * receive_messages(void * args) {
    response * msg = malloc(sizeof(response));
    int * thread_id  = (int *) args; 
	
	if (recv(tcp_sockets[*thread_id], msg, sizeof(response), 0) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    char action[10];
    if (msg->action == WRITE) {
        strcpy(action, "Escritor");
    } else if (msg->action == READ) {
        strcpy(action, "Lector");
    } else {
        printf("Invalid action %d\n", msg->action);
        exit(EXIT_FAILURE);
    }
    printf("[Cliente %d] %s, contador=%d, tiempo=%ld ns\n", *thread_id, action, msg->counter, msg->latency_time);
    
    free(msg);
	return NULL;
    
}

int send_request(request * req) {
    if (send(tcp_sockets[req->id],req, sizeof(request), 0) < 0) {
        perror("send");
        return -1;
    }
    return 0;
}
