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
#include "stub_server.h"



#define MAX_CLIENTS 600
#define PID_LENGTH 5
#define MAX_CONNECTIONS 1000
#define MAX_PUBLISHERS 100
#define MAX_SUBSCRIBERS 900
#define MAX_TOPICS 10
#define FALSE 0
#define TRUE 1

int NANO_MICRO = 1e3;
int SECS_NANO = 1e9;
int SECS_MILI = 1e3;

unsigned int clock_lamport = 0;

int tcp_socket;

int tcp_sockets [MAX_CONNECTIONS];
int client_counter = 0;
char Pid [PID_LENGTH];

int exit_flag = 0;

pthread_t threads_not_collected [MAX_CLIENTS];
int nthreads_not_collected = 0;

//int send_counter = 0;

int free_pos[MAX_CONNECTIONS];
publisher * publishers[MAX_PUBLISHERS];
subscriber * subscribers[MAX_SUBSCRIBERS];
int publishers_topics[MAX_TOPICS];
subscriber * subscribers_topics[MAX_TOPICS][MAX_SUBSCRIBERS];
char * topics[MAX_TOPICS];

int n_publishers = 0;
int n_subscribers = 0;
int n_topics = 0;


enum mode broker_mode = SEQ;

pthread_mutex_t mutex;//añadir uso del mutex para datos compartidos como subscribers_topics
pthread_mutex_t mutex_pub;
pthread_mutex_t mutex_sub;
pthread_barrier_t barrier;

sem_t semaphore;

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
		break;
	}
}

int init_Server(long port, char * mode) {
    
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
    if (mode != NULL) {
        if (strcmp(mode, "SEQ") == 0) broker_mode = SEQ;
        else if (strcmp(mode, "PARALLEL") == 0) broker_mode = PARALLEL;
        else if (strcmp(mode, "FAIR") == 0) broker_mode = FAIR;
    }
    //creamos un hilo que se encarge de aceptar las conexiones para poder volver al programa principal
    pthread_create(&thread_id, NULL, accept_connections, (void *) args);
    
    pthread_join(thread_id, NULL);
    
    return 0;
    }

// establish connections between processes
void * accept_connections(void * args) {
    pthread_t thread_ids[MAX_CLIENTS];
    threadArgs * arguments =(threadArgs *) args;
    struct sockaddr_in * server_addr = &arguments->server_addr;
    int * addrlen = &arguments->addrlen;

    signal(SIGINT, handler);
    int x = 0;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        free_pos[i] = TRUE;
    }

    while(!exit_flag) {
        int local_socket;
        
        if ((local_socket = accept(tcp_socket, (struct sockaddr *)server_addr,
         (socklen_t *)addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        int i = 0;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (free_pos[i] == TRUE) {
                free_pos[i] = FALSE;
                x = i;
                break;
            }   
        }

        recv_args * recv_Args = malloc(sizeof(recv_args));
        recv_Args->socket = local_socket;
        recv_Args->pos = x;


        //creamos un hilo por cada conexion que se quedar escchando indefinidadmente
        //sin interrumpir la ejecucion del hilo principal 
        pthread_create(&thread_ids[x], NULL, server_receive, recv_Args);

        
    }
    for (int i = 0; i < x; i++) {
        pthread_join(thread_ids[i], NULL);
    }

    free(args);
    
    return 0;  
}



void * server_receive(void *arg) {   
    
    recv_args * args = (recv_args *) arg;
    int socket_local = args->socket;
    int send_counter = 0;
    //int pos = args->pos;
    while(!exit_flag) {
        //printf("esperando mensaje\n"
    message * msg = malloc(sizeof(message));


    if (recv(socket_local, msg , sizeof(message), 0) == -1) {//comprobar si recibo 0 bytes
        perror("recv");
        exit(EXIT_FAILURE);
    }
    
    printf("action %d\n", msg->action);
    
    refresh_topics();
    printf("despues de refresh\n");

    if (msg->action == REGISTER_PUBLISHER){
        int id = register_publisher(msg, socket_local);
        response * res = malloc(sizeof(response));
        if (id == -1) {
            res->response_status = LIMIT;
        } else {
            res->response_status = OK;
            res->id = id;
            pthread_mutex_lock(&mutex);
            n_publishers++;
            pthread_mutex_unlock(&mutex);
        }

        if (send(socket_local, res, sizeof(response), 0) < 0) {
            perror("send");
            exit(EXIT_FAILURE);
        }
        free(res);
        
    }
    else if (msg->action == UNREGISTER_PUBLISHER){
        int id = unregister_publisher(msg->id);
        response * res = malloc(sizeof(response));
        if (id == -1) {
            res->response_status = ERROR;
        } else {
            
            res->response_status = OK;
            pthread_mutex_lock(&mutex_pub);
            n_publishers--;
            pthread_mutex_unlock(&mutex_pub);
        }
        res->id = id;

        if (send(socket_local, res, sizeof(response), 0) < 0) {
            perror("send");
            exit(EXIT_FAILURE);
        }
        free(res);
        close(socket_local);
        pthread_exit(NULL);

    }
    else if (msg->action == REGISTER_SUBSCRIBER){
        int id = register_subscriber(msg, socket_local);
        response * res = malloc(sizeof(response));
        if (id == -1) {
            res->response_status = LIMIT;
        } else {
            res->response_status = OK;
            pthread_mutex_lock(&mutex);
            n_subscribers++;
            pthread_mutex_unlock(&mutex);
        }
        res->id = id;
        printf("n_subscribers %d, id de nuevo sub %d  y fd   %d\n", n_subscribers, res->id, socket_local);


        if (send(socket_local, res, sizeof(response), 0) < 0) {
            perror("send");
            exit(EXIT_FAILURE);
        }
        free(res);
    }
    else if (msg->action == UNREGISTER_SUBSCRIBER){
        printf("msg->id %d\n", msg->id);
        int id = unregister_subscriber(msg->id);
        response * res = malloc(sizeof(response));
        if (id == -1) {
            res->response_status = ERROR;
        } else {
            res->response_status = OK;
            pthread_mutex_lock(&mutex_sub);
            n_subscribers--;
            pthread_mutex_unlock(&mutex_sub);
        }
        res->id = id;
        if (send(socket_local, res, sizeof(response), 0) < 0) {
            perror("send");
            exit(EXIT_FAILURE);
        }
        free(res);
        
        for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
            if (subscribers[i] != NULL) {
                printf("subscribers[i]->id %d\n", subscribers[i]->id);

            }
        }
        
        close(socket_local);
        pthread_exit(NULL);
    }
    else if (msg->action == PUBLISH_DATA){
        int j = 0;
        printf("Nuevo mensaje recibido en topic %s\n", msg->topic);
        pthread_mutex_lock(&mutex);
        for (j = 0; j < MAX_PUBLISHERS; j++) {
            if (topics[j] != NULL && strcmp(topics[j], msg->topic) == 0) {
                break;
            }
        }

        pthread_mutex_unlock(&mutex);
        printf("publish data despues de mmutex\n");
        pthread_mutex_lock(&mutex_sub);
        for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
            if (subscribers_topics[j][i] == NULL) {
                break;
            }
            printf("subscribers_topics[j][i]->id %d\n", subscribers_topics[j][i]->id);
            printf("fd de socket %d\n", subscribers_topics[j][i]->socket);
            
            send_counter++;
        
        }

        pthread_mutex_unlock(&mutex_sub);

        if (broker_mode == SEQ) {
            int null_counter = 0;
            pthread_mutex_lock(&mutex_sub);
            /* for (int k = 0; k < MAX_SUBSCRIBERS; k++) {
                if (subscribers_topics[j][k] == NULL) {
                    null_counter++;
                    if(null_counter == 10){
                        break;
                    }
                    continue;
                    break;
                }
                printf("subscribers_topics[j][k]->id %d\n", subscribers_topics[j][k]->id);
            
            }*/
            for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
                
                if (subscribers_topics[j][i] == NULL) {
                    printf ("no hay mas subscribers en este topic\n");
                    break;
                }
                printf("publico a id %d\n", subscribers_topics[j][i]->id);
                printf("fd de socket %d\n", subscribers_topics[j][i]->socket);
                publish * pub = malloc(sizeof(publish));
                pub->time_generated_data = msg->data.time_generated_data;
                strcpy(pub->data, msg->data.data);
                //printf("voy a publicar pub->data %s\n", pub->data);
                if (send(subscribers_topics[j][i]->socket, pub, sizeof(publish), 0) < 0) {
                    perror("send");
                    exit(EXIT_FAILURE);
                }
                free(pub);
                }
            pthread_mutex_unlock(&mutex_sub);
        }
        else if (broker_mode == PARALLEL) {
            pthread_t thread_ids[MAX_SUBSCRIBERS];
            int n_send = 0;
            pthread_mutex_lock(&mutex_sub);
            for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
                if (subscribers_topics[j][i] == NULL) {
                    break;
                }
                send_args * parallel_args = malloc(sizeof(send_args));
                publish * pub = malloc(sizeof(publish));
                pub->time_generated_data = msg->data.time_generated_data;
                strcpy(pub->data, msg->data.data);
                parallel_args->socket = subscribers_topics[j][i]->socket;
                parallel_args->data = pub;
                n_send++;
                pthread_create(&thread_ids[i], NULL, send_publish, (void *) parallel_args);
            }
            pthread_mutex_unlock(&mutex_sub);
            for (int i = 0; i < n_send; i++) {
                pthread_join(thread_ids[i], NULL);
            }
        }
        else if (broker_mode == FAIR) {
            pthread_t thread_ids[MAX_SUBSCRIBERS];
            int n_send = 0;
            pthread_barrier_init(&barrier, NULL, send_counter);
            
            pthread_mutex_lock(&mutex_sub);
            for (int i = 0; i < MAX_SUBSCRIBERS; i++) {
                
                if (subscribers_topics[j][i] == NULL) {
                    break;
                }
                send_args * parallel_args = malloc(sizeof(send_args));
                publish * pub = malloc(sizeof(publish));
                pub->time_generated_data = msg->data.time_generated_data;
                strcpy(pub->data, msg->data.data);
                parallel_args->socket = subscribers_topics[j][i]->socket;
                parallel_args->data = pub;
                n_send++;
                pthread_create(&thread_ids[i], NULL, send_publish, (void *) parallel_args);
            }
            pthread_mutex_unlock(&mutex_sub);
            for (int i = 0; i < n_send; i++) {
                pthread_join(thread_ids[i], NULL);
            }

            send_counter = 0;
        }
        else {
            printf("Invalid mode\n");
            exit(EXIT_FAILURE);
        }
    }
    else {
        printf("Invalid action %d\n", msg->action);
        exit(EXIT_FAILURE);
    }
    free(msg);
    }
    pthread_exit(NULL);
    
}

int register_publisher(message * msg, int socket) {
    int i = 0;
    int j = 0;
    publisher * pub = malloc(sizeof(publisher));
    if (n_publishers == MAX_PUBLISHERS) {
        printf("No hay espacio para mas publishers\n");
        return -1;
    }
    /*printf("topics al inicio\n");
    for (int k = 0; k < MAX_TOPICS; k++) {
        printf("topic %s\n", topics[k]);
    }*/
    pthread_mutex_lock(&mutex_pub);
    for (i = 0; i < MAX_PUBLISHERS; i++) {
        if (publishers[i] == NULL) {
            pub->id = i;
            strcpy(pub->topic, msg->topic);
            pub->socket = socket;
            publishers[i] = pub;
            pthread_mutex_lock(&mutex);
            for (j = 0; j < MAX_TOPICS; j++) {
                
                if (topics[j] == NULL) {
                    n_topics++;
                    printf("pub->topic %s\n", pub->topic);
                    topics[j] = malloc(sizeof(char) * 100);
                    strcpy(topics[j], pub->topic);
                    printf("topic[j] %s\n", topics[j]);
                    publishers_topics[j]++;
                    break;
                }
                if (strcmp(topics[j],pub->topic) == 0) {
                    publishers_topics[j]++;
                    break;
                }
            }
            pthread_mutex_unlock(&mutex);

            if (j == MAX_TOPICS) {
                printf("No hay espacio para mas topics\n");
                pthread_mutex_unlock(&mutex_pub);
                return -1;
            }
            
            printf(" Nuevo cliente (%d) Publicador conectado: %s\n", i, msg->topic);
            break;
        }
    }
    /*printf("topics al final\n");
    for (int k = 0; k < MAX_TOPICS; k++) {
        printf("topic %s\n", topics[k]);
    }*/
    printf("\n");
    if (i == MAX_PUBLISHERS) {
        printf("No hay espacio para mas publishers\n");
        pthread_mutex_unlock(&mutex_pub);
        return -1;
    }
    pthread_mutex_unlock(&mutex_pub);
    return i;
}

int register_subscriber(message * msg, int socket) {
    int i = 0;
    int j = 0;
    subscriber * sub = malloc(sizeof(subscriber));
    
    pthread_mutex_lock(&mutex_sub);
    for (int k = 0; k < MAX_SUBSCRIBERS; k++) {
        if (subscribers[k] == NULL) {
            sub->id = k + 1;
            strcpy(sub->topic, msg->topic);
            sub->socket = socket;
            subscribers[k] = sub;
            break;
        }
    } 
    if (n_subscribers == MAX_SUBSCRIBERS) {
        printf("No hay espacio para mas subscribers\n");
        pthread_mutex_unlock(&mutex_sub);
        return -1;
    }
    
    for (j = 0; j < MAX_TOPICS; j++) {   
        //printf("topic %s\n", topics[j]); 
        if (topics[j] != NULL && strcmp(topics[j],sub->topic) == 0) {
            for(i = 0; i < MAX_SUBSCRIBERS; i++) {
                if (subscribers_topics[j][i] == NULL) {
                    subscribers_topics[j][i] = sub;
                    printf(" Nuevo cliente (%d) Suscriptor conectado: %s\n", i, msg->topic);
                    break;
                }
            }
            i++;
            if (i == MAX_SUBSCRIBERS) {
                printf("No hay espacio para mas subscribers en este topic\n");
                pthread_mutex_unlock(&mutex_sub);
                return -1;
            }
            break;
        }
    }
    pthread_mutex_unlock(&mutex_sub);
    if (j == MAX_TOPICS) {
        printf("topic deseado no existe\n");
        return -1;
    }
    //printf("subscriber_topcs[j][i]->id en register%d\n",subscribers_topics[j][i-1]->id);
    return sub->id;
}


int unregister_publisher(int id) {
    int i = 0;
    printf("estoy en el unregister antes de mutex_pub\n");
    pthread_mutex_lock(&mutex_pub);
    for(i = 0; i < MAX_PUBLISHERS; i++) {
        if (publishers[i] != NULL && publishers[i]->id == id) {
            //int id = publishers[i]->id;
            printf("estoy en el unregister antes de mutex general\n");
            pthread_mutex_lock(&mutex);
            for (int j = 0; j < MAX_TOPICS; j++) {

                if (topics[j] == NULL) {
                    continue;
                }
                if (strcmp(publishers[i]->topic, topics[j]) == 0) {
                    publishers_topics[j]--;
                    printf("publishers_topics[j] %d\n", publishers_topics[j]);
                    printf("topic %s\n", topics[j]); 
                    break;   
                }
            }
            printf("estoy en el unregister despues de mutex general y voy a desbloquearlo\n");
            pthread_mutex_unlock(&mutex);
            //printf ("hemos encontrado el publisher\n");
            
            publishers[i] = NULL;
            pthread_mutex_unlock(&mutex_pub);
            //printf("publisher a null\n");
            return id;
        }
    }
    pthread_mutex_unlock(&mutex_pub);

    printf("No existe el publisher con id %d\n", id);
    return -1;
}

int unregister_subscriber(int id) {
    int i = 0;
    int j = 0;
    int k = 0;
    
    //printf("id %d\n", id);
    pthread_mutex_lock(&mutex_sub);
    for(i = 0; i < MAX_SUBSCRIBERS; i++) {
        if (subscribers[i] != NULL && subscribers[i]->id == id) {break;}   
    } 
    
    if (i == MAX_SUBSCRIBERS - 1) {//REVISAR
        printf("No existe el subscriber con id %d\n", id);
        return -1;
    }
    pthread_mutex_lock(&mutex);
    for (j = 0; j < MAX_TOPICS; j++) {   
        if (strcmp(subscribers[i]->topic, topics[j]) == 0) {
            break;
        }
    }    
    pthread_mutex_unlock(&mutex);
    if (j == MAX_TOPICS - 1) {
        printf("No existe el topic %s\n", subscribers[i]->topic);
        return -1;
    }
    //printf("j %d\n", j);
    for (k = 0; k < MAX_SUBSCRIBERS; k++) {
        
        if (subscribers_topics[j][k] == NULL) {
            printf("No existe el subscriber con id %d en el topic %s\n", id, subscribers[i]->topic);
            return -1;
        }
        //printf("k %d\n", k);
        
        if (subscribers_topics[j][k]->id == id) {
            subscribers_topics[j][k] = NULL;
            subscribers[i] = NULL;
            break;
        }
    }
    //printf("encontrado sub en topic\n");
                    
    if (k == MAX_SUBSCRIBERS - 1) {
        printf("No existe el subscriber con id %d en el topic %s\n", id, subscribers[i]->topic);
        //return -1;
    }
   
    for (k = k; k < MAX_SUBSCRIBERS; k++) {
        
        if (subscribers_topics[j][k+1] != NULL) {
            subscribers_topics[j][k] = subscribers_topics[j][k + 1];
        }
       
        if (subscribers_topics[j][k+1] == NULL) {
            subscribers_topics[j][k] = NULL;
            break;
        }
        
    }
    pthread_mutex_unlock(&mutex_sub);
    //printf("he actualizo la lista\n");
    for (int x = 0; x < MAX_SUBSCRIBERS; x++) {
        if (subscribers_topics[j][x] != NULL) {
            printf("subscribers_topics[j][k]->id %d\n", subscribers_topics[j][x]->id);

        }
    }
    //printf("lista actualizada\n");
    return id;
}

void * send_publish(void * arg) {
    send_args * args = (send_args *) arg;
    publish * pub = args->data;

    printf("voy a publicar %s\n", pub->data);
    if (broker_mode == FAIR) {
        pthread_barrier_wait(&barrier);
    }

    if (send(args->socket, pub, sizeof(publish), 0) < 0) {
        perror("send");
        exit(EXIT_FAILURE);
    }
    
    free(args);
    free(pub);
    return NULL;
}

void refresh_topics() {
    printf("refresh_topics\n");
    pthread_mutex_lock(&mutex);
    printf("rt mutex\n");
    for (int i = 0; i < MAX_TOPICS; i++) {
        //printf("publisher_topic %d\n", publishers_topics[i] );
        if(topics[i] == NULL) {
            continue;
        }
        if (publishers_topics[i] == 0 && subscribers_topics[i][0] == NULL) {
            printf("eliminando topic %s\n", topics[i]);
            free(topics[i]);
            topics[i] = NULL;
        }
    }
    pthread_mutex_unlock(&mutex);
}
