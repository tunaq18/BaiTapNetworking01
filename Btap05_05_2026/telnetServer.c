#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

int check_login(char *user, char *pass) {
    FILE *f = fopen("users.txt", "r");
    if (!f) return 0;

    char u[100], p[100];
    while (fscanf(f, "%s %s", u, p) != EOF) {
        if (strcmp(user, u) == 0 && strcmp(pass, p) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

void handle_client(int client_sock) {
    char buf[BUF_SIZE];
    char user[100], pass[100];

   while (1) {
    send(client_sock, "Username: ", 10, 0);
    recv(client_sock, user, sizeof(user), 0);

    send(client_sock, "Password: ", 10, 0);
    recv(client_sock, pass, sizeof(pass), 0);

    user[strcspn(user, "\r\n")] = 0;
    pass[strcspn(pass, "\r\n")] = 0;

    if (check_login(user, pass)) {
        send(client_sock, "Login success\n", 14, 0);
        break;
    } else {
        send(client_sock, "Login failed\n", 13, 0);
    }
}

    while (1) {
        send(client_sock, "cmd> ", 5, 0);
        int n = recv(client_sock, buf, sizeof(buf)-1, 0);
        if (n <= 0) break;
        buf[n] = 0;
        buf[strcspn(buf, "\r\n")] = 0;

        if (strcmp(buf, "exit") == 0) break;

        char command[BUF_SIZE];
        snprintf(command, sizeof(command), "%s > out.txt", buf);

        system(command);

        FILE *f = fopen("out.txt", "r");
        if (!f) continue;

        while (fgets(buf, sizeof(buf), f)) {
            send(client_sock, buf, strlen(buf), 0);
        }
        fclose(f);
    }

    close(client_sock);
    exit(0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(server_sock, (struct sockaddr*)&server, sizeof(server));
    listen(server_sock, 5);

    printf("Server running on port %d...\n", PORT);

    while (1) {
        client_sock = accept(server_sock, (struct sockaddr*)&client, &len);

        if (fork() == 0) {
            close(server_sock);
            handle_client(client_sock);
        } else {
            close(client_sock);
        }
    }

    return 0;
}