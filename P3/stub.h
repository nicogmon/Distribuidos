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
WRITE = 0,
READ
};
enum mode{
    WRITER = 0,
    READER
};

struct request {
enum operations action;
unsigned int id;
};
typedef struct request request;

struct response {
enum operations action;
unsigned int counter;
long latency_time;
};
typedef struct response response;

struct communication_info {
    int tcp_socket;
    char Pid[5];
};
typedef struct communication_info communication_info;

struct threadArgs {
    
    struct sockaddr_in  server_addr;
    int  addrlen;
};
typedef struct threadArgs threadArgs;

int  init_Server(long port, enum mode priority);

void *  accept_connections(void * args);

int init_Client(char * ip, long port, int id);

int send_request(request * req);

int send_response(response res, int arg_socket);



void * receive_messages(void * args);

void * server_receive(void *arg);