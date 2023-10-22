#include "stub.h"

#define ID "P1"

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
        printf("Incorrect arguments\n, usage: ./P1 <IP> <port>\n");
        return 1;
    }

    

    if (*endptr != '\0' && *endptr != '\n') {
        printf("Incorrect format port.\n");
        return 1;
    }

    init(2, char_ip, port, ID);

    int result = ready_to_shutdown();
    
    while (get_clock_lamport() < 1)
    {
        usleep(100);
        continue;   
    }
   
    waiting_order();
    
    while (get_clock_lamport() < 5)
    {
        usleep(100);
        continue;   
    }
    result = shutdown_ack();

    exit(result);




}