#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 512

typedef struct {
    char mssv[20];
    char hoten[100];
    char ngaysinh[20];
    float dtb;
} Student;

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    Student sv;

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

    // Nhập thông tin sinh viên
    printf("Nhap MSSV: ");
    fgets(sv.mssv, sizeof(sv.mssv), stdin);
    sv.mssv[strcspn(sv.mssv, "\n")] = '\0';

    printf("Nhap ho ten: ");
    fgets(sv.hoten, sizeof(sv.hoten), stdin);
    sv.hoten[strcspn(sv.hoten, "\n")] = '\0';

    printf("Nhap ngay sinh: ");
    fgets(sv.ngaysinh, sizeof(sv.ngaysinh), stdin);
    sv.ngaysinh[strcspn(sv.ngaysinh, "\n")] = '\0';

    printf("Nhap diem trung binh: ");
    scanf("%f", &sv.dtb);

    // Tạo socket TCP
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

    // Kết nối server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Khong ket noi duoc den server");
        close(sockfd);
        return 1;
    }

    // Gửi dữ liệu sinh viên sang server
    if (send(sockfd, &sv, sizeof(sv), 0) < 0) {
        perror("Gui du lieu that bai");
        close(sockfd);
        return 1;
    }

    printf("\nDa gui thong tin sinh vien den server thanh cong.\n");

    close(sockfd);
    return 0;
}