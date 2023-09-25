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
#include <errno.h>

#define PORT 6000

int exit_flag = 0;

void
handler(int number)
 {
switch (number) {
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
  
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //Meter ips si no no funciona desde otra maquina 
    server_addr.sin_port = htons(PORT);

    int addrlen = sizeof(server_addr);

    char buffer[1024] = { 0 };
    char* hello = "Hello client";
    char *path = (char *)malloc(sizeof(char) * 1024);

    int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    int new_socket;

    if (tcp_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully created...\n");
    const int enable = 1;
    if (setsockopt(tcp_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
        perror("setsockopt(SO_REUSEADDR) failed");

    if (bind (tcp_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        perror("bind failure");
        exit(EXIT_FAILURE);
    }
    printf("Socket successfully binded...\n");

    if(listen(tcp_socket, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server listening…\n");

    if((new_socket = accept(tcp_socket, (struct sockaddr *) &server_addr, (socklen_t*)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    int rec = recv (new_socket, buffer, sizeof(buffer), MSG_DONTWAIT);

    if(rec > 0){
        printf("+++%s\n", buffer);
    }
    printf(">");
    while (fgets(path, 100, stdin) != NULL) {
        if (send(new_socket, path, strlen(path), 0) < 0) {
            perror("send");
            
        }
            
        memset(buffer, 0, sizeof(buffer));
        if (recv(new_socket, buffer, 1024, MSG_DONTWAIT) > 0) {//comprobar si el error es diferente de nd que leer
            printf("+++%s\n", buffer);
        }
        
        printf(">");
        signal(SIGINT, handler);
        if (exit_flag == 1){
            printf("Exiting...\n");
            close(tcp_socket);
            exit(EXIT_SUCCESS);
        }
        
    }


    

    
    return 0;
}