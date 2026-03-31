/*******************************************************************************
* @file    Exercise01.c
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <errno.h>

#define MAX_CLIENTS 64

typedef struct {
    int fd;
    int state; // 0: nhap ten, 1: nhap mssv
    char name[128];
} Client;

// Hàm tạo email
void create_email(char *name_input, char *mssv_input, char *output) {
    char name[128];
    strcpy(name, name_input);

    // ===== xử lý tên =====
    char *words[10];
    int count = 0;

    char *token = strtok(name, " ");
    while (token && count < 10) {
        words[count++] = token;
        token = strtok(NULL, " ");
    }

    if (count < 2 || strlen(mssv_input) <= 2) {
        strcpy(output, "Du lieu khong hop le!\n");
        return;
    }

    // tên (từ cuối)
    char *ten = words[count - 1];

    // chữ cái đầu họ + đệm
    char initials[10] = "";
    for (int i = 0; i < count - 1; i++) {
        char c = words[i][0];
        if (c >= 'A' && c <= 'Z') c += 32;
        int len = strlen(initials);
        initials[len] = c;
        initials[len + 1] = 0;
    }

    // tên viết thường
    char ten_lower[64];
    strcpy(ten_lower, ten);
    for (int i = 0; ten_lower[i]; i++) {
        if (ten_lower[i] >= 'A' && ten_lower[i] <= 'Z')
            ten_lower[i] += 32;
    }

    // MSSV bỏ 2 số đầu
    char *mssv_cut = mssv_input + 2;

    // tạo email
    sprintf(output, "%s.%s%s@sis.hust.edu.vn\n",
            ten_lower, initials, mssv_cut);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // non-blocking listener
    unsigned long ul = 1;
    ioctl(listener, FIONBIO, &ul);

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        close(listener);
        return 1;
    }

    if (listen(listener, 5)) {
        perror("listen() failed");
        close(listener);
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    Client clients[MAX_CLIENTS];
    int nclients = 0;

    char buf[256];

    while (1) {
        // accept client
        int client = accept(listener, NULL, NULL);
        if (client != -1) {
            printf("New client: %d\n", client);

            clients[nclients].fd = client;
            clients[nclients].state = 0;

            ul = 1;
            ioctl(client, FIONBIO, &ul);

            char *msg = "Nhap ho ten: ";
            send(client, msg, strlen(msg), 0);

            nclients++;
        }

        // xử lý client
        for (int i = 0; i < nclients; i++) {
            int fd = clients[i].fd;

            int len = recv(fd, buf, sizeof(buf) - 1, 0);

            if (len > 0) {
                buf[len] = 0;

                // bỏ newline
                buf[strcspn(buf, "\r\n")] = 0;

                if (clients[i].state == 0) {
                    // nhận tên
                    strcpy(clients[i].name, buf);
                    clients[i].state = 1;

                    char *msg = "Nhap MSSV: ";
                    send(fd, msg, strlen(msg), 0);
                }
                else if (clients[i].state == 1) {
                    // nhận mssv → tạo email
                    char email[256];
                    create_email(clients[i].name, buf, email);

                    send(fd, email, strlen(email), 0);

                    // reset để tiếp tục dùng
                    clients[i].state = 0;
                    char *msg = "Nhap ho ten: ";
                    send(fd, msg, strlen(msg), 0);
                }
            }
            else if (len == 0) {
                // client disconnect
                printf("Client %d disconnected\n", fd);
                close(fd);

                for (int j = i; j < nclients - 1; j++) {
                    clients[j] = clients[j + 1];
                }
                nclients--;
                i--;
            }
            else {
                if (errno != EWOULDBLOCK) {
                    close(fd);

                    for (int j = i; j < nclients - 1; j++) {
                        clients[j] = clients[j + 1];
                    }
                    nclients--;
                    i--;
                }
            }
        }
    }

    close(listener);
    return 0;
}