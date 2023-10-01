#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <signal.h>

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
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    int addrlen = sizeof(server_addr);

    char buffer[1024] = { 0 };
    char* hello = "Hello client";
    char *path = (char *)malloc(sizeof(char) * 100);

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    int new_socket;

    if (tcp_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    const int enable = 1;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (bind (tcp_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        perror("bind failure");
        exit(EXIT_FAILURE);
    }

    if(listen(tcp_socket, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if((new_socket = accept(tcp_socket, (struct sockaddr *) &server_addr, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    int rec = recv (new_socket, buffer, sizeof(buffer), 0);

    if (rec == -1) {
        perror("recv failure");
        exit(EXIT_FAILURE);
    }
    if (rec > 0) {
        printf("+++%s\n", buffer);
        printf(">");
        while (fgets(path, 100, stdin) != NULL) {
            if (sig_c == 1) {
                printf("Cerrando cliente...\n");
                close(tcp_socket);
                free(path);
                exit(EXIT_SUCCESS);
            }
            if (send(new_socket, path, strlen(path), 0) < 0) {
                perror("send");
                exit(EXIT_FAILURE);
            }

            memset(buffer, 0, sizeof(buffer));
            if (recv(new_socket, buffer, 1024, 0) < 0) {
                perror("recv");
                exit(EXIT_FAILURE);
            }
            printf("+++%s\n", buffer);
            printf(">");

            
           
            
        }
    }

    
    return 0;
}