#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int server_fd,client_fd;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];
    // Tạo socket TCP
    server_fd = socket(AF_INET,SOCK_STREAM,0);
    // Thiết lập thông tin server
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server, sizeof(server));
    listen(server_fd, 1);

    client_fd =accept(server_fd, NULL, NULL);
    int n;
    while( (n=recv(client_fd,buffer,sizeof(buffer)-1,0))>0){
        buffer[n] = '\0'; // Đảm bảo chuỗi kết thúc bằng null
        printf("%s",buffer);
    }
    close(client_fd);
    close(server_fd);
    return 0;
}