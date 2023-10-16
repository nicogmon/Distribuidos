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

unsigned int clock_lamport = 0;

int tcp_socket;

int tcp_socket1;
int tcp_socket3;

int tcp_sockets [2];
int MAX_CLIENTS = 3;
char Pid [5];
int i = 0;
//no comparten memoria cada uno tiene su reloj no p1 p2 p3 sino cada uno tiene su reloj
int init(int flag, char * ip, long port, char * id){
    strcpy(Pid, id);
    printf("Pid %s\n", Pid);
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
    int tcp_socket;
    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	//Meter ips si no no funciona desde otra maquina 
	server_addr.sin_port = htons(port);

    int addrlen = sizeof(server_addr);
    
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully created...\n");

    const int enable = 1;
    printf("%d\n",tcp_socket);
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
    printf("Socket successfully binded...\n");

    if (listen(tcp_socket, 5) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
    printf("Server listening…\n");
   
    accept_connections(tcp_socket, &server_addr, &addrlen);
    
    return 0;
    }

int init_Client(char * ip, long port){
    
    if ((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "socket creado %d\n", tcp_socket);
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
    
    return tcp_socket;
}

// establish connections between processes
void * accept_connections(int tcp_socket, struct sockaddr_in * server_addr, int * addrlen){
    pthread_t thread_ids[2];
    printf("Accepting connections...\n");
    
    for (int i = 0; i < 2; i++){
        if ((tcp_sockets[i] = accept(tcp_socket, (struct sockaddr *)server_addr, (socklen_t *)addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Connection accepted...\n");
        pthread_create(&thread_ids[i], NULL, server_receive,NULL);
    }
    return 0;  
}

int ready_to_shutdown(){
    message msg_out = {0};
    msg_out.action = READY_TO_SHUTDOWN;
    update_clock(NULL, &msg_out);
    //msg_out.clock_lamport = get_clock_lamport();
    strcpy(msg_out.origin,Pid);

    send_message(&msg_out,0);
    
    return 0;
}

int shutdown_now(int dest){
    message msg_out;
    msg_out.action = SHUTDOWN_NOW;
    update_clock(NULL, &msg_out);
    //msg_out.clock_lamport = get_clock_lamport();
    strcpy(msg_out.origin,Pid);

    send_message(&msg_out, dest);
    return 0;
}

int shutdown_ack(){
    message msg_out;
    msg_out.action = SHUTDOWN_ACK;
    update_clock(NULL, &msg_out);
    //msg_out.clock_lamport = get_clock_lamport();
    strcpy(msg_out.origin,Pid);

    send_message(&msg_out, 0);
    return 0;
}

// send a message to a remote process
int send_message(message * msg, int arg_socket){
    printf("Sending message...\n");
    
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
    return 0;
}

// receive a message from a remote process
message * receive_messages(){
    message * msg = malloc(sizeof(message));
	
	if (recv(tcp_socket, msg, sizeof(message), 0) > 0) {	//comprobar si el error es diferente de nd que leer
		printf(">%s, %u, RECV (%s), %d\n",Pid, 2, msg->origin, READY_TO_SHUTDOWN);
	}
    update_clock(msg, NULL);

    free(msg);
	return NULL;
    
    return 0;
}

void * client_receive(void *arg)
{
    
	message * msg = malloc(sizeof(message));
	
	if (recv(tcp_socket, msg, sizeof(message), 0) > 0) {	//comprobar si el error es diferente de nd que leer
		printf(">%s, %u, RECV (%s), %d\n",Pid, 2, msg->origin, READY_TO_SHUTDOWN);
	}
    update_clock(msg, NULL);

    free(msg);
	return NULL;
}
void * server_receive(void *arg)
{   
    int first = 0;
    printf("Server receive\n");
    int socket_local = tcp_sockets[i];
    printf("Socket local %d\n", socket_local);
    i = i + 1;
    while(1){
        message * msg = malloc(sizeof(message));
        if (recv(socket_local, msg, sizeof(message), 0) > 0) {	//comprobar si el error es diferente de nd que leer
            
		    printf(">%s, %u, RECV (%s), %d\n",Pid, 2, msg->origin,msg->action );
            update_clock(msg, NULL); 
            fprintf(stderr, "Server receive despues de update clock%d\n", get_clock_lamport());
	    }
        if (first == 0){
            if (strcmp(msg->origin, "P1") == 0){
                tcp_socket1 = socket_local;
                fprintf(stderr, "tcp_socket1 %d\n" ,tcp_socket1);
            }
            else if (strcmp(msg->origin, "P3") == 0){
                tcp_socket3 = socket_local;
                fprintf(stderr, "tcp_socket2 %d\n" ,tcp_socket1);
            }
            first = 1;
        }
        
        free(msg);

    }


   
}

// update Lamport clock value
void update_clock(message* msg_in, message* msg_out){

    if (msg_out != NULL){
        printf("clock msg out antes %d\n", clock_lamport);
        clock_lamport++;
        msg_out->clock_lamport = clock_lamport;
        printf("clock msg out  %d\n", msg_out->clock_lamport);
    }
    else {
        
        fprintf(stderr, "clock msg in antes  %d\n", clock_lamport );
        clock_lamport = get_max(clock_lamport, msg_in->clock_lamport) + 1 ;
        fprintf(stderr, "clock msg in desp %d\n", clock_lamport);
    }
    return;
}
int get_clock_lamport(){
    //fprintf(stderr, "clock en get  %d\n", clock_lamport);
    return clock_lamport;
}
// get current Lamport clock value


int get_max(int a, int b){
    printf("a %d b %d\n", a, b);
    if (a > b){
        return a;
    }
    else {
        return b;
    }
}
