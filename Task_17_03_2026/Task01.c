#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>         // close()
#include <arpa/inet.h>      // socket(), inet_pton(), sockaddr_in
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Kiểm tra số lượng tham số dòng lệnh
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

    // Tạo socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Khong tao duoc socket");
        return 1;
    }

    // Thiết lập thông tin server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("Dia chi IP khong hop le");
        close(sockfd);
        return 1;
    }

    // Kết nối đến server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Khong ket noi duoc den server");
        close(sockfd);
        return 1;
    }

    printf("Da ket noi den server %s:%d\n", server_ip, port);
    printf("Nhap du lieu tu ban phim de gui den server (nhap 'exit' de thoat):\n");

    // Đọc từ bàn phím và gửi đến server
    while (1) {
        printf("> ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        // Thoát nếu nhập exit
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }

        // Gửi dữ liệu đến server
        if (send(sockfd, buffer, strlen(buffer), 0) < 0) {
            perror("Loi gui du lieu");
            break;
        }
    }

    close(sockfd);
    printf("Da dong ket noi.\n");

    return 0;
}