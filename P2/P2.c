
#include "stub.h"


int 
main(int argc, char *argv[])
{
   
    char * endptr;
    char my_id[5] = "P2";
    long port = strtol(argv[1], &endptr, 10);

    

    init(1, NULL, port, my_id);
    fprintf(stderr, "Server initialized\n");

    //message * msg1 = receive_message(1);
    while (get_clock_lamport() < 3)
    {
        continue; 
    }
    
    shutdown_now(1);
    
    while (get_clock_lamport() < 7)
    {
     continue;   
    }
    shutdown_now(3);

    while (get_clock_lamport() < 11)
    {
     continue;   
    }
    printf("Los clientes fueron correctamente apagados en t(lamport) = %d\n", get_clock_lamport());
    return 0;
}