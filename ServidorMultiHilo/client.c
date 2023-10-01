#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define PORT 6000

int exit_flag = 0;

void
handler(int number)
 {
switch ( number ) {
case SIGINT:
    fprintf(stderr, "SIGINT received!\n" );
    exit_flag = 1;
    break;
}
}

int
main(int argc, char *argv[])
{

    char *endptr;
	int * p = malloc(sizeof(int));
    char *id = argv[1];
    
    char *char_ip = argv[2];

    long port = strtol(argv[3], &endptr, 10);


    if (*endptr != '\0' && *endptr != '\n') {
        printf("No se pudo convertir a entero.\n");
        return 1;
    }

    setbuf(stdout, NULL);
    

    char buffer[1024] = { 0 };
    char *msg = (char *)malloc(sizeof(char) * 1024);
    sprintf(msg,"Hello server! From client: %s\n", id);

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

   
    struct in_addr ipv4Addr;

    if (inet_pton(AF_INET, char_ip, &ipv4Addr) == 1) {
        printf("Dirección IPv4 configurada: %s\n", inet_ntoa(ipv4Addr));
    } else {
        perror("Error al configurar la dirección IPv4");
        return 1;
    }

    //in_addr_t ip = inet_addr(ipv4Addr.s_addr);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = ipv4Addr.s_addr;
    server_addr.sin_port = htons(port); 
    

    int addrlen = sizeof(server_addr);
    
    if(connect(tcp_socket, (struct sockaddr *) &server_addr, addrlen) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    if (send(tcp_socket, msg, strlen(msg), 0) < 0) {
    perror("send");
    exit(EXIT_FAILURE);
    }

    memset(buffer, 0, sizeof(buffer));
    if (recv(tcp_socket, buffer, 1024, 0) < 0) {
                perror("recv");
                exit(EXIT_FAILURE);
    }
    printf(">%s\n", buffer);
            
    
    
    printf(">");
    signal(SIGINT, handler);
    if (exit_flag == 1){
        printf("Exiting...\n");
        close(tcp_socket);
        exit(EXIT_SUCCESS);
    }
    
}    
        
    

    
   
 
