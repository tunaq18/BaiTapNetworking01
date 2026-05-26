#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));

    listen(server, 5);

    printf("Server running at port %d...\n", PORT);

    while (1) {
        int client = accept(server, NULL, NULL);

        char request[4096];
        int len = recv(client, request, sizeof(request) - 1, 0);

        if (len <= 0) {
            close(client);
            continue;
        }

        request[len] = '\0';

        printf("Request:\n%s\n", request);

        int a = 0, b = 0;
        char op[10] = "";
        int found = 0;

        // GET
        if (strncmp(request, "GET", 3) == 0) {

            found = sscanf(
                request,
                "GET /?a=%d&b=%d&op=%s",
                &a, &b, op
            );
        }

        // POST
        else if (strncmp(request, "POST", 4) == 0) {

            char *body = strstr(request, "\r\n\r\n");

            if (body != NULL) {
                body += 4;

                found = sscanf(
                    body,
                    "a=%d&b=%d&op=%s",
                    &a, &b, op
                );
            }
        }

        float result = 0;
        char message[100];

        if (found == 3) {

            switch (op[0]) {
                case '+':
                    result = a + b;
                    sprintf(message,
                            "%d + %d = %.2f",
                            a, b, result);
                    break;

                case '-':
                    result = a - b;
                    sprintf(message,
                            "%d - %d = %.2f",
                            a, b, result);
                    break;

                case '*':
                    result = a * b;
                    sprintf(message,
                            "%d * %d = %.2f",
                            a, b, result);
                    break;

                case '/':
                    if (b == 0)
                        strcpy(message,
                               "Loi: chia cho 0");
                    else {
                        result = (float)a / b;

                        sprintf(message,
                                "%d / %d = %.2f",
                                a, b, result);
                    }
                    break;

                default:
                    strcpy(message,
                           "Toan tu khong hop le");
            }
        }
        else {
            strcpy(message,
                   "Thieu tham so");
        }

        char body[1024];

        sprintf(body,
                "<html>\n"
                "<head>\n"
                "<title>Calculator</title>\n"
                "</head>\n"
                "<body style='text-align:center'>"
                "<h1>Calculator Result</h1>\n"
                "<h2>%s</h2>"
                "</body>\n"
                "</html>\n",
                message);

        char response[2048];

        sprintf(response,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %ld\r\n"
                "Connection: close\r\n"
                "\r\n%s",
                strlen(body),
                body);

        send(client, response, strlen(response), 0);

        close(client);
    }

    close(server);

    return 0;
}