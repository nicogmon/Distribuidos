#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#define PORT 8080


int
main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
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
            if (send(tcp_socket, path, strlen(path), 0) < 0) {
            perror("send");
            exit(EXIT_FAILURE);
            }
            
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