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
    setbuf(stdout, NULL);
    signal(SIGINT, handler);

    char buffer[1024] = { 0 };
    char *path = (char *)malloc(sizeof(char) * 1024);
    char ip[INET_ADDRSTRLEN];

    strcpy(ip, "192.168.1.31");

    struct in_addr ipv4Addr;


    if (inet_pton(AF_INET, ip, &ipv4Addr) == 1) {
        printf("Dirección IPv4 configurada: %s\n", inet_ntoa(ipv4Addr));
    } else {
        perror("Error al configurar la dirección IPv4");
        return 1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = ipv4Addr.s_addr;
    server_addr.sin_port = htons(PORT);
    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
     

    int addrlen = sizeof(server_addr);
    
    if(connect(tcp_socket, (struct sockaddr *) &server_addr, addrlen) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }


    printf(">");
    while (fgets(path, 1024, stdin) != NULL) {
            
            if (send(tcp_socket, path, strlen(path), 0) < 0) {
            perror("send");
            exit(EXIT_FAILURE);
            }

            fd_set readmask;
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
            }
            
            printf(">");
            
            if (exit_flag == 1){
                printf("Exiting...\n");
                free(path);
                close(tcp_socket);
                exit(EXIT_SUCCESS);
            }
    }
}    
        
    

    
   
 
