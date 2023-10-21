#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>		
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <math.h>
#include "stub.h"


#define MAX_CLIENTS 2 

unsigned int clock_lamport = 0;

int tcp_socket;

int tcp_socket1;
int tcp_socket3;

int tcp_sockets [2];

char Pid [5];
int i = 0;

pthread_t recv_thread;
//no comparten memoria cada uno tiene su reloj no p1 p2 p3 sino cada uno tiene su reloj
int init(int flag, char * ip, long port, const char * id){
    strcpy(Pid, id);
    //printf("Pid %s\n", Pid);
    if (flag == 1){
        init_Server(port);
    }
    else if (flag == 2){
        init_Client(ip, port);
    }
    return 0;
}

int init_Server(long port){
    
    setbuf(stdout, NULL);
    
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//Meter ips si no no funciona desde otra maquina 
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
    

    if (listen(tcp_socket, 5) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
    
    pthread_t thread_id;
    threadArgs  * args = (threadArgs *) malloc(sizeof(threadArgs));
    args->server_addr = &server_addr;
    args->addrlen = &addrlen;

    
    pthread_create(&thread_id, NULL, accept_connections, (void *) args);
    //accept_connections( &server_addr, &addrlen);
    
    return 0;
    }

// establish connections between processes
void * accept_connections(void * args){
    pthread_t thread_ids[2];
    

    threadArgs * arguments =(threadArgs *) args;
    struct sockaddr_in * server_addr = arguments->server_addr;
    int * addrlen = arguments->addrlen;
        
    for (int i = 0; i < MAX_CLIENTS; i++){
        if ((tcp_sockets[i] = accept(tcp_socket, (struct sockaddr *)server_addr, (socklen_t *)addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        pthread_create(&thread_ids[i], NULL, server_receive,NULL);
    }
    free(args);
    for (int i = 0; i < MAX_CLIENTS; i++){
        //fprintf(stderr, "thread id %ld\n", thread_ids[i]);
        pthread_join(thread_ids[i], NULL);
    }
    
    
    return 0;  
}

void * server_receive(void *arg)
{   
    int first = 0;
    int flag = 1;
    int socket_local = tcp_sockets[i];
    i = i + 1;

    while(flag){
        message * msg = malloc(sizeof(message));
        if (recv(socket_local, msg, sizeof(message), 0) > 0) {	//comprobar si el error es diferente de nd que leer
             update_clock(msg, NULL); 
		    fprintf(stderr,">%s, %u, RECV (%s), %s\n",Pid, clock_lamport, msg->origin, get_action(msg->action));
	    }
        if (msg->action == SHUTDOWN_ACK){
            flag = 0;
            
        }
        if (first == 0){
            if (strcmp(msg->origin, "P1") == 0){
                tcp_socket1 = socket_local;
            }
            else if (strcmp(msg->origin, "P3") == 0){
                tcp_socket3 = socket_local;
            }
            first = 1;
        }
        
        free(msg);

    }

   
}


// receive a message from a remote process


int init_Client(char * ip, long port){
    
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
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

    if(connect(tcp_socket, (struct sockaddr *) &server_addr, addrlen) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    //meter thread para escuchar todo el rato 
    return tcp_socket;
}

int client_receive()
{
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, receive_messages,NULL);
	pthread_join(thread_id, NULL);
	return 0;
}

void * receive_messages(){
    message * msg = malloc(sizeof(message));
	
	if (recv(tcp_socket, msg, sizeof(message), 0) > 0) {	//comprobar si el error es diferente de nd que leer
		printf(">%s, %u, RECV (%s), %s\n",Pid, clock_lamport, msg->origin, get_action(msg->action));
	}
    update_clock(msg, NULL);

    free(msg);
	return NULL;
    
    return 0;
}

int ready_to_shutdown(){
    message msg_out = {0};
    msg_out.action = READY_TO_SHUTDOWN;
    update_clock(NULL, &msg_out);
    strcpy(msg_out.origin,Pid);

    send_message(&msg_out,0);
    
    return 0;
}

int shutdown_now(int dest){
    
    message msg_out = {0};
    msg_out.action = SHUTDOWN_NOW;
    update_clock(NULL, &msg_out);
    //msg_out.clock_lamport = get_clock_lamport();
    strcpy(msg_out.origin,Pid);

    send_message(&msg_out, dest);

    return 0;
}

int shutdown_ack(){
    
    message msg_out = {0};
    msg_out.action = SHUTDOWN_ACK;
    update_clock(NULL, &msg_out);
    //msg_out.clock_lamport = get_clock_lamport();
    strcpy(msg_out.origin,Pid);

    send_message(&msg_out, 0);
    return 0;
}

// send a message to a remote process
int send_message(message * msg, int arg_socket){
    
    if (arg_socket != 0){
        if (arg_socket == 1){
            tcp_socket = tcp_socket1;
        }
        else if (arg_socket == 3){
            tcp_socket = tcp_socket3;
        }
    }
    if (send(tcp_socket, msg, sizeof(message), 0) < 0) {
        perror("send");
        return -1;
    }
    printf(">%s, %u, SEND, %s\n",Pid, clock_lamport, get_action(msg->action));
    return 0;
}

// update Lamport clock value
void update_clock(message* msg_in, message* msg_out){

    if (msg_out != NULL){
        clock_lamport++;
        msg_out->clock_lamport = clock_lamport;
    }
    else {
        clock_lamport = get_max(clock_lamport, msg_in->clock_lamport) + 1 ;
    }
    return;
}

int get_clock_lamport(){
    return clock_lamport;
}
// get current Lamport clock value


int get_max(int a, int b){
    if (a > b){
        return a;
    }
    else {
        return b;
    }
}


char * get_action(int action){
    if (action == READY_TO_SHUTDOWN){
        return "READY_TO_SHUTDOWN";
    }
    else if (action == SHUTDOWN_NOW){
        return "SHUTDOWN_NOW";
    }
    else if (action == SHUTDOWN_ACK){
        return "SHUTDOWN_ACK";
    }
    else {
        return "ERROR";
    }
}