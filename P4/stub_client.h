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


void handler(int number);

int init_Client(char * ip, long port,  int client_type);

int register_pub_sub( char * topic);

int client_publish(char * data);

int unregister_pub_sub(int action);

int subscriber_recieve(void * args);

long double get_time();


