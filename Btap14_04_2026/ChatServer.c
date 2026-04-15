#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 13
#define BUFFER_SIZE 1024

typedef struct{
    int fd; 
    char name[50];
    int is_registered;
}Client;

void broadcast(Client clients[], int sender_idx, char *message, int num_clients) {
    char time_str[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y/%m/%d %I:%M:%S%p", t);

    char final_msg[BUFFER_SIZE + 100];
    sprintf(final_msg, "%s %s: %s\n", time_str, clients[sender_idx].name, message);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].fd != -1 && i != sender_idx && clients[i].is_registered) {
            send(clients[i].fd, final_msg, strlen(final_msg), 0);
        }
    }
}

int main(){
    int master_socket, addrlen, new_socket, activity, valread;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];
    Client clients[MAX_CLIENTS];

    // Khởi tạo addrlen
    addrlen = sizeof(address);

    // Khởi tạo mảng client
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].is_registered = 0;
    }

    // Tạo socket server
    master_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    
    // Thiết lập địa chỉ server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(master_socket, (struct sockaddr *)&address, sizeof(address));
    listen(master_socket, 3);

    printf("Server dang chay tren port %d...\n", PORT);

    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 0;
    
    while (1) {
        // Thiết lập mảng pollfd
        nfds = 0;
        fds[nfds].fd = master_socket;
        fds[nfds].events = POLLIN;
        nfds++;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd > 0) {
                fds[nfds].fd = clients[i].fd;
                fds[nfds].events = POLLIN;
                nfds++;
            }
        }

        activity = poll(fds, nfds, -1);

        //  Chấp nhận kết nối mới
        if (fds[0].revents & POLLIN) {
            new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            printf("Ket noi moi, socket fd: %d\n", new_socket);
            
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].fd == -1) {
                    clients[i].fd = new_socket;
                    char *msg = "Vui long nhap ten theo cu phap 'client_id: client_name':\n";
                    send(new_socket, msg, strlen(msg), 0);
                    break;
                }
            }
        }

        //  Kiểm tra dữ liệu từ các client cũ
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int sd = clients[i].fd;
            if (sd > 0) {
                // Tìm file descriptor trong mảng fds
                int found = 0;
                for (int j = 1; j < nfds; j++) {
                    if (fds[j].fd == sd && (fds[j].revents & POLLIN)) {
                        found = 1;
                        break;
                    }
                }
                if (!found) continue;
                memset(buffer, 0, BUFFER_SIZE);
                valread = read(sd, buffer, BUFFER_SIZE);

                if (valread == 0) { // Client ngắt kết nối
                    close(sd);
                    clients[i].fd = -1;
                    clients[i].is_registered = 0;
                    printf("Client ngat ket noi.\n");
                } else {
                    buffer[strcspn(buffer, "\r\n")] = 0; // Xóa ký tự xuống dòng

                    if (!clients[i].is_registered) {
                        // Xử lý đăng ký tên: "client_id: name"
                        char name_part[50];
                        if (sscanf(buffer, "client_id: %s", name_part) == 1) {
                            strcpy(clients[i].name, name_part);
                            clients[i].is_registered = 1;
                            char *ok_msg = "Dang ky thanh cong! Bay gio ban co the chat.\n";
                            send(sd, ok_msg, strlen(ok_msg), 0);
                        } else {
                            char *err_msg = "Sai cu phap! Yeu cau: 'client_id: client_name'\n";
                            send(sd, err_msg, strlen(err_msg), 0);
                        }
                    } else {
                        // Broadcast tin nhắn
                        broadcast(clients, i, buffer, MAX_CLIENTS);
                    }
                }
            }
        }
    }
    return 0;
}