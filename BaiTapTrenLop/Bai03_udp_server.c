#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int sockfd;
    struct sockaddr_in server_addr,client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t len = sizeof(client_addr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    printf("UDP server dang chay tren cong %d...\n", PORT);

    while(1){
        int n = recvfrom(sockfd,buffer,BUFFER_SIZE -1,0,(struct sockaddr *)&client_addr,&len);
        buffer[n]='\0';
        printf("Nhan du lieu tu client: %s\n",buffer);
        //echo lai du lieu cho client
        sendto(sockfd,buffer,n,0,(struct sockaddr *)&client_addr,len);
    }
    close(sockfd);
    return 0;
}