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
    REGISTER_PUBLISHER = 0,
    UNREGISTER_PUBLISHER,
    REGISTER_SUBSCRIBER,
    UNREGISTER_SUBSCRIBER,
    PUBLISH_DATA
};

enum status {
    ERROR = 0,
    LIMIT,
    OK
};

enum mode{
    SEQ = 0,
    PARALLEL,
    FAIR
};

struct publish {
    struct timespec time_generated_data;
    char data[100];
}; typedef struct publish publish;

struct message {
    enum operations action;
    char topic[100];
    // Solo utilizado en mensajes de UNREGISTER
    int id;
    // Solo utilizado en mensajes PUBLISH_DATA
    struct publish data;
};typedef struct message message;

struct response {
    enum status response_status;
    int id;
};  typedef struct response response;

struct recv_args {
    int socket;
    int pos;
};
typedef struct recv_args recv_args;

struct threadArgs {
    struct sockaddr_in  server_addr;
    int  addrlen;
};
typedef struct threadArgs threadArgs;

struct publisher {
    int id;
    char topic[100];
    int socket;
}; typedef struct publisher publisher;

struct send_args{
    int socket;
    publish * data;
}; typedef struct send_args send_args;


struct subscriber {
    int id;
    char topic[100];
    int socket;
}; typedef struct subscriber subscriber;

int  init_Server(long port, char * mode);

void *  accept_connections(void * args);

int send_response(response res, int arg_socket);

void * receive_messages(void * args);

void * server_receive(void *arg);

int register_publisher(message * msg, int arg_socket);

int unregister_publisher(int id);

int register_subscriber(message * msg, int arg_socket);

int unregister_subscriber(int id);

void * send_publish(void *arg);

void  refresh_topics();