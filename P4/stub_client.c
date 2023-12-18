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
#include "stub_client.h"


#define MAX_CLIENTS 600
#define PID_LENGTH 5
#define MAX_CONNECTIONS 1000
#define FALSE 0
#define TRUE 1

int NANO_MICRO = 1e3;
int SECS_NANO = 1e9;
int SECS_MILI = 1e3;

unsigned int clock_lamport = 0;

int tcp_socket; //cada proceso tiene su socket
int id;
char topic[100];
int type;



int counter = 0;




int init_Client(char * ip, long port, int client_type) {
    
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
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

    if (connect(tcp_socket, (struct sockaddr *) &server_addr, addrlen) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    type = client_type;
    printf("%s conectado con broker \n", type == REGISTER_PUBLISHER ? "Publisher" : "Subscriber");

    return tcp_socket;
}

int register_pub_sub( char * client_topic) {

    message * msg = malloc(sizeof(message));
    if (type == REGISTER_PUBLISHER) {
        msg->action = REGISTER_PUBLISHER;
    } else if (type == REGISTER_SUBSCRIBER){
        msg->action = REGISTER_SUBSCRIBER;
    } else {
        printf("Tipo de cliente incorrecto\n");
        return -1;
    }
    strcpy(msg->topic, client_topic);
    strcpy(topic, client_topic);
    if (send(tcp_socket, msg, sizeof(message), 0) < 0) {
        perror("send");
        return -1;
    }
    free(msg);
    response * res = malloc(sizeof(response));
    if (recv(tcp_socket, res, sizeof(response), 0) < 0) {
        perror("recv");
        return -1;
    }
    if (res->response_status == ERROR || res->response_status == LIMIT) {
        printf("Error al hacer el registro: error=%s\n", res->response_status == ERROR ? "ERROR" : "LIMIT");
        close(tcp_socket);
        return -1;
    }
    if (res->response_status == OK) {
        printf("Registrado correctamente con ID: %d para topic %s\n", res->id, topic);
    }
    id = res->id;

    free(res);
    return 0;
}

int client_publish(char * data) {

    publish * pub = malloc(sizeof(publish));
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    long double time_in_seconds = time.tv_sec + (time.tv_nsec / (long double) SECS_NANO);
    pub->time_generated_data = time;
    strcpy(pub->data, data);
    
    message * msg = malloc(sizeof(message));
    msg->action = PUBLISH_DATA;
    strcpy(msg->topic, topic);
    msg->data = *pub;

    if (send(tcp_socket, msg, sizeof(message), 0) < 0) {
        perror("send");
        return -1;
    }
    printf("[%ld.%ld] Publicado mensaje topic: %s - mensaje: %s - Generó:%LF\n", time.tv_sec, time.tv_nsec, topic, data, time_in_seconds);
    free(msg);
    free(pub);
    return 0;

}

int unregister_pub_sub(int action) {
    message * msg = malloc(sizeof(message));
    if (action == UNREGISTER_PUBLISHER) {
        msg->action = UNREGISTER_PUBLISHER;
    } else if ( action == UNREGISTER_SUBSCRIBER){
        msg->action = UNREGISTER_SUBSCRIBER;
    } else {
        printf("Tipo de cliente incorrecto\n");
        return -1;
    }
    strcpy(msg->topic, topic);
    msg->id = id;
    if (send(tcp_socket, msg, sizeof(message), 0) < 0) {
        perror("send");
        return -1;
    }
    free(msg);

    response * res = malloc(sizeof(response));
    if (recv(tcp_socket, res, sizeof(response), 0) < 0) {
        perror("recv");
        return -1;
    }
    if (res->response_status == ERROR) {
        printf("Error al hacer el De-registro\n");
        return -1;
    }
    if (res->response_status == OK) {
        printf("De-registrado %d correctamente del broker\n", res->id);
    }
    free(res);
    close(tcp_socket);
    return 0;
}

int subscriber_recieve(void * args) {

    //int socket = *((int *) args);
    //free(args);

    
    publish * pub = malloc(sizeof(publish));
    int bytes_read = 0;

    fd_set readmask;
    struct timeval timeout;
    FD_ZERO(&readmask); // Reset la mascara
    FD_SET(tcp_socket, &readmask); // Asignamos el nuevo descriptor
    //FD_SET(STDIN_FILENO, &readmask); // Entrada
    timeout.tv_sec=1; timeout.tv_usec=500000;

    if (select(tcp_socket+1, &readmask, NULL, NULL, &timeout)==-1)
               return 0;
    if (FD_ISSET(tcp_socket, &readmask)){
        bytes_read = recv(tcp_socket,pub,sizeof(publish), 0);
        
        if (bytes_read == 0) {
        return -1;
        }
        if(bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                perror("recv");
                return -1;
            }
        }
    
    }else{
        return 0;
    }
    
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    
    
    long double time_in_seconds = get_time();   
    long double time_generated = pub->time_generated_data.tv_sec + (pub->time_generated_data.tv_nsec / (long double) SECS_NANO);
    long double latency = time_in_seconds - time_generated;
    
    printf("[%ld.%ld] Recibido mensaje topic: %s - mensaje: %s - Generó: %LF - Recibido: %LF - Latencia: %0.6LF \n",
            time.tv_sec, time.tv_nsec, topic, pub->data,time_generated, time_in_seconds, latency );

    free(pub);
    
    return 0;
    
}

long double get_time() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    long double time_in_seconds = time.tv_sec + (time.tv_nsec / (long double) SECS_NANO);
    return time_in_seconds;
}

