#include "stub.h"
#include <getopt.h>






int main(int argc, char *argv[]) {
    long port = 0;
    
    enum mode priority = 0;
    int opt;
    struct option long_options[] = {
        {"port", required_argument, 0, 'a'},
        {"priority", required_argument, 0, 'b' },
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "a:b:", long_options, NULL)) != -1) {
            switch (opt) {
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
                        priority=  WRITER;
                    } else if (strcmp(optarg, "reader") == 0) {
                        priority = READER;
                    } else {
                        printf("Invalid priority %s, valid priorities write / reader \n", optarg);
                        exit(EXIT_FAILURE);
                    }
                    //printf("priority = %d\n", priority);
                    break;
                default:
                    printf("Usage: %s --port <port> --priority <priority>\n", argv[0]);
                    exit(EXIT_FAILURE);
            }
    }
    
    init_Server(port, priority);
    

    
}