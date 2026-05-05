#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/wait.h>

#define PORT 9090
#define BUF_SIZE 1024

// Hàm format thời gian
void format_time(char *out, char *format) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);

    if (strcmp(format, "dd/mm/yyyy") == 0)
        strftime(out, 100, "%d/%m/%Y", tm_info);
    else if (strcmp(format, "dd/mm/yy") == 0)
        strftime(out, 100, "%d/%m/%y", tm_info);
    else if (strcmp(format, "mm/dd/yyyy") == 0)
        strftime(out, 100, "%m/%d/%Y", tm_info);
    else if (strcmp(format, "mm/dd/yy") == 0)
        strftime(out, 100, "%m/%d/%y", tm_info);
    else
        strcpy(out, "ERROR: Invalid format\n");
}

void handle_client(int client) {
    char buf[BUF_SIZE];

    while (1) {
        int ret = recv(client, buf, sizeof(buf) - 1, 0);
        if (ret <= 0) break;

        buf[ret] = 0;
        buf[strcspn(buf, "\r\n")] = 0;

        printf("Client cmd: %s\n", buf);

        // Kiểm tra lệnh
        if (strncmp(buf, "GET_TIME", 8) != 0) {
            send(client, "ERROR: Invalid command\n", 23, 0);
            continue;
        }

        char cmd[50], format[50];
        int n = sscanf(buf, "%s %s", cmd, format);

        if (n != 2) {
            send(client, "ERROR: Missing format\n", 22, 0);
            continue;
        }

        char result[100];
        format_time(result, format);

        send(client, result, strlen(result), 0);
        send(client, "\n", 1, 0);
    }

    close(client);
    exit(0);
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(listener, (struct sockaddr*)&server, sizeof(server));
    listen(listener, 5);

    printf("Time server running on port %d...\n", PORT);

    while (1) {
        // tránh zombie
        while (waitpid(-1, NULL, WNOHANG) > 0);

        int client = accept(listener, NULL, NULL);
        if (client < 0) continue;

        int pid = fork();

        if (pid == 0) {
            close(listener);
            handle_client(client);
        } else {
            close(client);
        }
    }

    return 0;
}