#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    int fd;
    int is_logged_in;
} Client;

// Hàm kiểm tra tài khoản từ file text
int check_login(char *user, char *pass) {
    FILE *f = fopen("database.txt", "r");
    if (f == NULL) return 0;

    char f_user[50], f_pass[50];
    while (fscanf(f, "%s %s", f_user, f_pass) != EOF) {
        if (strcmp(user, f_user) == 0 && strcmp(pass, f_pass) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

// Hàm thực thi lệnh và gửi trả kết quả
void execute_command(int client_fd, char *command) {
    char full_cmd[BUFFER_SIZE + 20];
    // Chuyển hướng kết quả lệnh vào file out.txt
    sprintf(full_cmd, "%s > out.txt 2>&1", command);
    system(full_cmd);

    // Đọc file out.txt và gửi lại cho client
    FILE *f = fopen("out.txt", "r");
    if (f) {
        char file_buf[BUFFER_SIZE];
        while (fgets(file_buf, BUFFER_SIZE, f) != NULL) {
            send(client_fd, file_buf, strlen(file_buf), 0);
        }
        fclose(f);
    }
}

int main() {
    int master_socket, new_socket;
    struct sockaddr_in address;
    Client clients[MAX_CLIENTS];
    char buffer[BUFFER_SIZE];

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].is_logged_in = 0;
    }

    master_socket = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(master_socket, (struct sockaddr *)&address, sizeof(address));
    listen(master_socket, 5);

    printf("Telnet Server dang chay tren port %d...\n", PORT);

    fd_set readfds;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(master_socket, &readfds);
        int max_sd = master_socket;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd > 0) FD_SET(clients[i].fd, &readfds);
            if (clients[i].fd > max_sd) max_sd = clients[i].fd;
        }

        select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(master_socket, &readfds)) {
            new_socket = accept(master_socket, NULL, NULL);
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == -1) {
                    clients[i].fd = new_socket;
                    send(new_socket, "Nhap user pass (vi du: admin admin): ", 38, 0);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].fd;
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = read(sd, buffer, BUFFER_SIZE);
                
                if (valread <= 0) {
                    close(sd);
                    clients[i].fd = -1;
                    clients[i].is_logged_in = 0;
                } else {
                    buffer[strcspn(buffer, "\r\n")] = 0;

                    if (!clients[i].is_logged_in) {
                        char user[50], pass[50];
                        if (sscanf(buffer, "%s %s", user, pass) == 2 && check_login(user, pass)) {
                            clients[i].is_logged_in = 1;
                            send(sd, "Dang nhap thanh cong! Nhap lenh: \n", 35, 0);
                        } else {
                            send(sd, "Sai tai khoan! Nhap lai: ", 25, 0);
                        }
                    } else {
                        // Da dang nhap, thuc thi lenh
                        execute_command(sd, buffer);
                        send(sd, "\nDone. Nhap lenh tiep theo: ", 28, 0);
                    }
                }
            }
        }
    }
    return 0;
}