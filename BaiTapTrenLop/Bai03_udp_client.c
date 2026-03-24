#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUF_SIZE 1024

int main(){
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];

    sockfd= socket(AF_INET,SOCK_DGRAM,0);

    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(PORT);
    server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

    printf("Nhap du lieu can gui: ");

    while(1){
        fgets(buffer,BUF_SIZE,stdin);
        buffer[strcspn(buffer,"\n")]='\0';

        sendto(sockfd,buffer,strlen(buffer),0,(struct sockaddr *)&server_addr,sizeof(server_addr));

        int n =recvfrom(sockfd,buffer,BUF_SIZE -1,0,NULL,NULL);
        buffer[n]='\0';
        printf("Nhan du lieu tu server: %s\n",buffer);
    }
    close(sockfd);
    return 0;
}