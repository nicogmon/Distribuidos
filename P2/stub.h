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

enum operations {
    READY_TO_SHUTDOWN = 0,
    SHUTDOWN_NOW,
    SHUTDOWN_ACK
};

struct message {
    char origin[20];
    enum operations action;
    unsigned int clock_lamport;
}; 
typedef struct message message;

struct communication_info {
    int tcp_socket;
    char Pid[5];
};
typedef struct communication_info communication_info;

struct threadArgs {
    int tcp_socket;
    struct sockaddr_in * server_addr;
    int * addrlen;
    int id;
};
typedef struct threadArgs threadArgs;

// initialize communication channels
int init(int flag, char * ip, long port, char * id);

int  init_Server(long port);

void *  accept_connections(void * args);

int init_Client(char * ip, long port);

int send_message(message * msg, int arg_socket);

message * receive_messages();
void * client_receive(void *arg);
void * server_receive(int client);
// update Lamport clock value
void update_clock(message* msg_in, message* msg_out);

// get current Lamport clock value
int get_clock_lamport();

int ready_to_shutdown();

int shutdown_now(int dest);

int shutdown_ack();