#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

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
    FILE *fp;
    int port;
    char *log_file;
    char client_ip[INET_ADDRSTRLEN];
    int bytes_received;

    if (argc != 3) {
        fprintf(stderr, "Cach dung: %s <cong> <ten_file_log>\n", argv[0]);
        return 1;
    }

    port = atoi(argv[1]);
    log_file = argv[2];

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Cong khong hop le.\n");
        return 1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Khong tao duoc socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind that bai");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen that bai");
        close(server_fd);
        return 1;
    }

    printf("Server dang lang nghe tai cong %d...\n", port);

    fp = fopen(log_file, "a");
    if (fp == NULL) {
        perror("Khong mo duoc file log");
        close(server_fd);
        return 1;
    }

    while (1) {
        client_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("Accept that bai");
            continue;
        }

        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));

        bytes_received = recv(client_fd, &sv, sizeof(sv), 0);
        if (bytes_received < 0) {
            perror("Nhan du lieu that bai");
            close(client_fd);
            continue;
        }

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char time_str[30];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);

        printf("\nThong tin sinh vien nhan duoc:\n");
        printf("IP        : %s\n", client_ip);
        printf("Thoi gian : %s\n", time_str);
        printf("MSSV      : %s\n", sv.mssv);
        printf("Ho ten    : %s\n", sv.hoten);
        printf("Ngay sinh : %s\n", sv.ngaysinh);
        printf("DTB       : %.2f\n", sv.dtb);

        fprintf(fp, "%s %s %s %s %s %.2f\n",
                client_ip,
                time_str,
                sv.mssv,
                sv.hoten,
                sv.ngaysinh,
                sv.dtb);
        fflush(fp);

        close(client_fd);
    }

    fclose(fp);
    close(server_fd);
    return 0;
}