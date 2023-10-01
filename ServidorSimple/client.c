#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>


#define PORT 8080

int sig_c = 0;

void
handler(int number)
 {
switch ( number ) {
    case SIGINT:
        fprintf(stderr, "SIGINT received!\n" );
        sig_c = 1;
        break;
    case SIGTERM:
        fprintf(stderr, "SIGTERM received! \n" );
        break;
    }
}

int
main(int argc, char *argv[])
{
    signal(SIGINT, handler);
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

    int addrlen = sizeof(server_addr);

    char buffer[1024] = { 0 };
    char* hello = "Hello server";
    char *path = (char *)malloc(sizeof(char) * 100);

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    
    if(connect(tcp_socket, (struct sockaddr *) &server_addr, addrlen) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf(">");
    while (fgets(path, 100, stdin) != NULL) {
             if (sig_c == 1) {
                printf("Cerrando servidor...\n");
                close(tcp_socket);
                free(path);
                exit(EXIT_SUCCESS);
            }
            if (send(tcp_socket, path, strlen(path), 0) < 0) {
            perror("send");
            exit(EXIT_FAILURE);
            }

            memset(buffer, 0, sizeof(buffer));
            if (recv(tcp_socket, buffer, 1024, 0) < 0) {
                perror("recv");
                exit(EXIT_FAILURE);
            }
            printf("+++%s\n", buffer);
            printf(">");

           
    
        }
    

    
    //int valread = read(tcp_socket, buffer, 1024);
    
    close(tcp_socket);
    return 0;
}