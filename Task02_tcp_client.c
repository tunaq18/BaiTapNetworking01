#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    if (argc != 3) {
        fprintf(stderr, "Cach dung: %s <dia chi IP> <cong>\n", argv[0]);
        return 1;
    }

    char *server_ip = argv[1];
    int port = atoi(argv[2]);

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Cong khong hop le.\n");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Khong tao duoc socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Dia chi IP khong hop le");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Khong ket noi duoc den server");
        close(sockfd);
        return 1;
    }

    printf("Da ket noi den server %s:%d\n", server_ip, port);

    // Nhận câu chào từ server
    bytes_received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        printf("Cau chao tu server: %s\n", buffer);
    }

    printf("Nhap du lieu gui den server (nhap 'exit' de thoat):\n");

    while (1) {
        printf("> ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }

        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            perror("Loi gui du lieu");
            break;
        }
    }

    close(sockfd);
    printf("Da dong ket noi.\n");

    return 0;
}