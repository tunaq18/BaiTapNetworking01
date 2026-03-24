#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[BUFFER_SIZE];
    int bytes_received;

    if (argc != 4) {
        fprintf(stderr, "Cach dung: %s <cong> <tep_chua_cau_chao> <tep_luu_noi_dung_client>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    char *greeting_file = argv[2];
    char *output_file = argv[3];

    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Cong khong hop le.\n");
        return 1;
    }

    FILE *fp_greet = fopen(greeting_file, "r");
    if (fp_greet == NULL) {
        perror("Khong mo duoc tep chua cau chao");
        return 1;
    }

    FILE *fp_out = fopen(output_file, "a");
    if (fp_out == NULL) {
        perror("Khong mo duoc tep luu noi dung client");
        fclose(fp_greet);
        return 1;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Khong tao duoc socket");
        fclose(fp_greet);
        fclose(fp_out);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind that bai");
        close(server_fd);
        fclose(fp_greet);
        fclose(fp_out);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen that bai");
        close(server_fd);
        fclose(fp_greet);
        fclose(fp_out);
        return 1;
    }

    printf("Server dang lang nghe tai cong %d...\n", port);

    client_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Accept that bai");
        close(server_fd);
        fclose(fp_greet);
        fclose(fp_out);
        return 1;
    }

    printf("Da co client ket noi.\n");

    // Đọc nội dung câu chào từ file và gửi cho client
    while (fgets(buffer, BUFFER_SIZE, fp_greet) != NULL) {
        if (send(client_fd, buffer, strlen(buffer), 0) < 0) {
            perror("Gui cau chao that bai");
            close(client_fd);
            close(server_fd);
            fclose(fp_greet);
            fclose(fp_out);
            return 1;
        }
    }

    // Nhận dữ liệu client gửi đến và ghi vào file
    while ((bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        fputs(buffer, fp_out);
    }

    if (bytes_received < 0) {
        perror("Loi nhan du lieu");
    } else {
        printf("Da nhan xong du lieu tu client.\n");
    }

    close(client_fd);
    close(server_fd);
    fclose(fp_greet);
    fclose(fp_out);

    printf("Server da dong.\n");
    return 0;
}