
#include "stub.h"


int
main(int argc, char *argv[])
{

    char *endptr;
    
    char *char_ip = argv[1];
    long port = strtol(argv[2], &endptr, 10);

    char id[5] = "P3";

    if (*endptr != '\0' && *endptr != '\n') {
        printf("No se pudo convertir a entero.\n");
        return 1;
    }

    init(2, char_ip, port, id);
    printf("Client initialized\n");


    int result = ready_to_shutdown();
    printf("P1 esta listo para apagarse clock %d\n", get_clock_lamport());
    while (get_clock_lamport() < 1)
    {
        continue;   
    }
    fprintf(stderr, "clock a 1 en P3\n");
    receive_messages();
    fprintf(stderr, "clock en P3 %d\n", get_clock_lamport());
    while (get_clock_lamport() < 5)
    {
        continue;   
    }
    result = shutdown_ack();
    
    
    exit(EXIT_SUCCESS);
}