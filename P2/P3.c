#include "stub.h"

#define ID "P3"

int
main(int argc, char *argv[])
{

    char *endptr;
    argc--;
    argv++;
    char *char_ip = argv[0];
    argc--;
    argv++;
    long port = strtol(argv[0], &endptr, 10);
    argc--;
    argv++;

    if (argc != 0) {
        printf("Incorrect arguments\n, usage: ./P3 <IP> <port>\n");
        return 1;
    }

    if (*endptr != '\0' && *endptr != '\n') {
        printf("Incorrect format port.\n");
        return 1;
    }

    init( char_ip, port, ID);

    int result = ready_to_shutdown();
    
    while (get_clock_lamport() < 1)
    {
        //sleep de 500 microsegundos para evitar usar el 100% de la CPU
        //pero mantener una espera activa para poder actualizar el clock
        usleep(500);
        continue;   
    }
    
    waiting_order();
    
    while (get_clock_lamport() < 9)
    {
        usleep(500);
        continue;   
    }

    result = shutdown_ack();
    shutdown_machine();
    
    
    exit(result);
}