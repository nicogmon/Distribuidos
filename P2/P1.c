
#include "stub.h"


int
main(int argc, char *argv[])
{

    char *endptr;
    
    char *char_ip = argv[1];
    long port = strtol(argv[2], &endptr, 10);

    char id[5] = "P1";

    if (*endptr != '\0' && *endptr != '\n') {
        printf("No se pudo convertir a entero.\n");
        return 1;
    }

    init(2, char_ip, port, id);

    ready_to_shutdown();

    while (get_clock_lamport() < 1)
    {
        continue;   
    }
    receive_messages();
    while (get_clock_lamport() < 9)
    {
        continue;   
    }
    shutdown_ack();

    exit(EXIT_SUCCESS);




}