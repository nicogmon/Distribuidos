#include "stub.h"
#include <getopt.h>

enum mode mode;

enum operations get_operation(enum mode mode) {
    
    if (mode == WRITER) {
        return WRITE;
    } else if (mode == READER){
        return READ;
    } else {
        printf("Invalid mode %d\n", mode);
        exit(EXIT_FAILURE);
    }
}

void * thread_actions( void * args) {
    request * req = malloc(sizeof(request));
    req->action = get_operation(mode);
    
    req->id = *(int *) args;
    printf("Sending request %d\n", req->id);
    send_request(req);
    receive_messages(args);
}

int main(int argc, char *argv[]) {
    int nthreads = 0;
    long port = 0;
    char ip[12];
    
    
    if (argc < 9) {
        printf("Usage: %s --ip <ip> --port <port> --mode <mode> --threads <threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int opt;
    struct option long_options[] = {
        {"ip", required_argument, 0, 'i'},
        {"port", required_argument, 0, 'a'},
        {"mode", required_argument, 0, 'b' },
        {"threads", required_argument, 0, 't'},
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "i:a:b:t:", long_options, NULL)) != -1) {
            switch (opt) {
                case 'i':
                    //printf("ip = %s\n", optarg);
                    strcpy(ip, optarg);
                    break;
                case 'a':
                    char *endptr;
                    port = strtol(optarg, &endptr,10);
                    if (*endptr != '\0' && *endptr != '\n') {
                        printf("Incorrect format port\n");
                        return 1;
                    }
                    //printf("port = %ld\n", port);
                    break;
                case 'b':
                    if (strcmp(optarg, "writer") == 0) {
                        mode = WRITER;
                    } else if (strcmp(optarg, "reader") == 0) {
                        mode = READER;
                    } else {
                        printf("Invalid mode %s, valid modes write / reader \n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    //printf("mode = %d\n", mode);
                    break;
                case 't':
                    if ((nthreads = atoi(optarg)) == 0 || nthreads < 0){
                        fprintf(stderr, "Invalid number of threads\n");
                        exit(EXIT_FAILURE);
                    }
                    //printf("threads = %d\n", nthreads);
                    break;
                default:
                    printf("Usage: %s --port <port> --priority <priority>\n", argv[0]);
                    exit(EXIT_FAILURE);
            }


    }
    
    pthread_t threads[nthreads];

    for (int i = 0; i < nthreads; i++) {
        init_Client(ip, port, i );
        void * args = malloc(sizeof(int));
        *(int *) args = i;
        pthread_create(&threads[i], NULL, thread_actions, args);
    }

    for(int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }
}