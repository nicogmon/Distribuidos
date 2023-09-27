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

    in_addr_t ip = inet_addr(char_ip);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(ip);
    server_addr.sin_port = htons(PORT); 
    

    int addrlen = sizeof(server_addr);
    
    if(connect(tcp_socket, (struct sockaddr *) &server_addr, addrlen) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    
    if (send(tcp_socket, msg, strlen(msg), 0) < 0) {
    perror("send");
    exit(EXIT_FAILURE);
    }

    if (recv(tcp_socket, buffer, 1024, 0) < 0) {
                perror("recv");
                exit(EXIT_FAILURE);
    }
    printf(">%s\n", buffer);
            
    /*fd_set readmask;
    struct timeval timeout;
    FD_ZERO(&readmask); // Reset la mascara
    FD_SET(tcp_socket, &readmask); // Asignamos el nuevo descriptor
    FD_SET(STDIN_FILENO, &readmask); // Entrada
    timeout.tv_sec=1; timeout.tv_usec=500000;

    if (select(tcp_socket+1, &readmask, NULL, NULL, &timeout)==-1)
        exit(-1);
    
    memset(buffer, 0, sizeof(buffer));
    if (FD_ISSET(tcp_socket, &readmask)){
        recv(tcp_socket, buffer, 1024, 0);
        printf("+++%s\n", buffer);      
    }*/
    
    printf(">");
    signal(SIGINT, handler);
    if (exit_flag == 1){
        printf("Exiting...\n");
        close(tcp_socket);
        exit(EXIT_SUCCESS);
    }
    
}    
        
    

    
   
 