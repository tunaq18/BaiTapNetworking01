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
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];

    // Tạo socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        exit(EXIT_FAILURE);
    }
    // Thiết lập địa chỉ server
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    // Kết nối đến server
    if(connect(sockfd,(struct sockaddr *)&server, sizeof(server))<0){
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Nhap du lieu (Ctrl+D de ket thuc):\n");
    while(1){
        fgets(buffer,BUFFER_SIZE,stdin);
        buffer[strcspn(buffer,"\n")]='\0';
        send(sockfd,buffer,strlen(buffer),0);
    }
    close(sockfd);
    return 0;
}