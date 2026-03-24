#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct {
    char mssv[20];
    char hoten[100];
    char ngaysinh[20];
    float dtb;
} Student;

int main(int argc, char *argv[]) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    Student sv;
    int bytes_received;

    if (argc != 2) {
        fprintf(stderr, "Cach dung: %s <cong>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Cong khong hop le.\n");
        return 1;
    }

    // Tạo socket TCP
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Khong tao duoc socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Gắn socket với địa chỉ IP và cổng
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind that bai");
        close(server_fd);
        return 1;
    }

    // Chờ kết nối từ client
    if (listen(server_fd, 5) < 0) {
        perror("Listen that bai");
        close(server_fd);
        return 1;
    }

    printf("Server dang lang nghe tai cong %d...\n", port);

    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Accept that bai");
        close(server_fd);
        return 1;
    }

    printf("Da co client ket noi.\n");

    // Nhận dữ liệu sinh viên
    bytes_received = recv(client_fd, &sv, sizeof(sv), 0);
    if (bytes_received < 0) {
        perror("Nhan du lieu that bai");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    // In thông tin sinh viên nhận được
    printf("\nThong tin sinh vien nhan duoc:\n");
    printf("MSSV      : %s\n", sv.mssv);
    printf("Ho ten    : %s\n", sv.hoten);
    printf("Ngay sinh : %s\n", sv.ngaysinh);
    printf("DTB       : %.2f\n", sv.dtb);

    close(client_fd);
    close(server_fd);

    return 0;
}