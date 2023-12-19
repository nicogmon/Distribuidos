#include "stub_client.h"
#include <getopt.h>

int exit_flag = 0;

void handler(int number)
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


int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    signal(SIGINT, handler);
    char ip[12];
    long port = 0;
    char *topic;

    if (argc < 7) {
        printf("Usage: ./publisher --ip $BROKER_IP --port $BROKER_PORT --topic $TOPIC\n");
        exit(EXIT_FAILURE);
    }

    int opt;
    struct option long_options[] = {
        {"ip", required_argument, 0, 'a'},
        {"port", required_argument, 0, 'b' },
        {"topic", required_argument, 0, 'c' },
        {0, 0, 0, 0}
    };
    while ((opt = getopt_long(argc, argv, "a:b:c:", long_options, NULL)) != -1) {
            switch (opt) {
                case 'a':
                    strcpy(ip, optarg);
                    break;
                    
                case 'b':
                    char *endptr;
                    port = strtol(optarg, &endptr,10);
                    if (*endptr != '\0' && *endptr != '\n') {
                        printf("Incorrect format port\n");
                        return 1;
                    }
                    break;
                case 'c':
                    topic = optarg;
                    break;

                default:
                    printf("Usage: ./publisher --ip $BROKER_IP --port $BROKER_PORT --topic $TOPIC\n");
                    exit(EXIT_FAILURE);
            }
    }
    init_Client(ip, port, REGISTER_PUBLISHER);

    int status = register_pub_sub(topic);

    if (status == -1) {
        //printf("Error al registrarse\n");
        exit(EXIT_FAILURE);
    }
    char buffer[100];
    
    sleep(2);
    for (int i = 0; i < 100; i++) {
        FILE  * file = fopen ("/proc/loadavg", "r");
        if (file == NULL) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            client_publish(buffer);
        }
        fclose(file);
        
        usleep(100000);
    }

    status = unregister_pub_sub(UNREGISTER_PUBLISHER);

    exit(EXIT_SUCCESS);
}