#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>  

#define PORT 8080

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(listener, (struct sockaddr*)&server, sizeof(server));
    listen(listener, 5);

    printf("Server running on port %d...\n", PORT);

    while (1) {
        while (waitpid(-1, NULL, WNOHANG) > 0);

        int client = accept(listener, NULL, NULL);
        if (client < 0) continue;

        printf("New client connected: %d\n", client);

        int pid = fork();

        if (pid == 0) {
            // child process
            close(listener);

            char buf[1024];
            int ret = recv(client, buf, sizeof(buf) - 1, 0);

            if (ret > 0) {
                buf[ret] = 0;
                printf("PID %d received:\n%s\n", getpid(), buf);
            }

            char *msg =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n\r\n"
                "<html><body><h1>Xin chao cac ban</h1></body></html>";

            send(client, msg, strlen(msg), 0);

            close(client);
            printf("Client handled by PID: %d\n", getpid());

            exit(0);
        }
        else if (pid > 0) {
            // parent process
            close(client);
        }
    }

    close(listener);
    return 0;
}