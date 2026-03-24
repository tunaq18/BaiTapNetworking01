#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Tạo socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    // Thiết lập địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // Kết nối đến server
    if(connect(sockfd,(struct sockaddr *)&server_addr, sizeof(server_addr))<0){
        perror("connect");
        exit(EXIT_FAILURE);
    }

    while(fgets(buffer, sizeof(buffer), stdin) != NULL){
        send(sockfd, buffer, strlen(buffer), 0);
    }
    close(sockfd);
    return 0;
}