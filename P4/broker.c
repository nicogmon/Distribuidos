#include "stub_server.h"
#include <getopt.h>



int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    
    long port = 0;
    char *mode;

    if (argc < 5) {
        printf("Usage: %s --port <port> --priority <priority>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int opt;
    struct option long_options[] = {
        {"port", required_argument, 0, 'a' },
        {"mode", required_argument, 0, 'b' },
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
                    break;
                case 'b':
                    mode = optarg;
                    break;

                default:
                    printf("Usage: %s --port <port> --priority <priority>\n", argv[0]);
                    exit(EXIT_FAILURE);
            }
    }
    init_Server(port, mode);
}