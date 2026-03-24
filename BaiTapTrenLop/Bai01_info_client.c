#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Tạo socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Khong tao duoc socket");
        return 1;
    }
    // Thiết lập thông tin server
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    DIR *d = opendir(".");
    struct dirent *dir;
    struct stat st;
    while((dir = readdir(d)) != NULL){
        if(stat(dir->d_name, &st)==0 && S_ISREG(st.st_mode)){
            sprintf(buffer, "%s|%ld bytes\n", dir->d_name,st.st_size);
            send(sockfd, buffer, strlen(buffer), 0);
        }
    }
    close(sockfd);
    return 0;
}