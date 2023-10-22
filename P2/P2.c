#include "stub.h"

const char ID[] = "P2";

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
        printf("Incorrect arguments\n, usage: ./P2 <IP> <port>\n");
        return 1;
    }

    if (*endptr != '\0' && *endptr != '\n') {
        printf("Incorrect format port\n");
        return 1;
    }

    init(char_ip, port, ID);
    
    while (get_clock_lamport() < 3)
    {
        usleep(500);
        continue; 
    }
    
    shutdown_now(1);

    while (get_clock_lamport() < 7)
    {    
        usleep(500);
        continue;   
     
    }

    shutdown_now(3);

    while (get_clock_lamport() < 11)
    {   
        usleep(500);
        continue;   
    }
    printf("Los clientes fueron correctamente apagados en t(lamport) = %d\n", get_clock_lamport());
    shutdown_machine();
    return 0;
}