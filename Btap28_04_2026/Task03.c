#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 9000
#define MAX_CLIENTS 30
#define MAX_TOPICS 10
#define BUFFER_SIZE 1024

typedef struct
{
    int sock;
    char topics[MAX_TOPICS][50];
    int topic_count;
} Client;

Client clients[MAX_CLIENTS];

// Kiểm tra client có subscribe topic không
int is_subscribed(Client *c, char *topic)
{
    for (int i = 0; i < c->topic_count; i++)
    {
        if (strcmp(c->topics[i], topic) == 0)
            return 1;
    }
    return 0;
}

// Thêm topic cho client
void subscribe(Client *c, char *topic)
{
    if (is_subscribed(c, topic))
        return;
    if (c->topic_count < MAX_TOPICS)
    {
        strcpy(c->topics[c->topic_count++], topic);
    }
}

// Hủy đăng ký topic cho client
void unsubscribe(Client *c, char *topic)
{
    for (int i = 0; i < c->topic_count; i++)
    {
        if (strcmp(c->topics[i], topic) == 0)
        {
            // dồn mảng
            for (int j = i; j < c->topic_count - 1; j++)
            {
                strcpy(c->topics[j], c->topics[j + 1]);
            }
            c->topic_count--;
            return;
        }
    }
}

// Gửi message đến các client đã SUB
void publish(char *topic, char *msg)
{
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "[%s] %s\n", topic, msg);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i].sock != 0 &&
            is_subscribed(&clients[i], topic))
        {
            send(clients[i].sock, buffer, strlen(buffer), 0);
        }
    }
}

int main()
{
    int server_fd, new_socket, addrlen;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE];

    fd_set readfds;

    // init clients
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i].sock = 0;
        clients[i].topic_count = 0;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    printf("Server dang chay port %d...\n", PORT);

    addrlen = sizeof(address);

    while (1)
    {
        FD_ZERO(&readfds);

        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = clients[i].sock;
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // có client mới
        if (FD_ISSET(server_fd, &readfds))
        {
            new_socket = accept(server_fd,
                                (struct sockaddr *)&address,
                                (socklen_t *)&addrlen);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i].sock == 0)
                {
                    clients[i].sock = new_socket;
                    clients[i].topic_count = 0;
                    break;
                }
            }

            printf("New client connected\n");
        }

        // xử lý client
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = clients[i].sock;

            if (FD_ISSET(sd, &readfds))
            {
                int valread = recv(sd, buffer, BUFFER_SIZE - 1, 0);

                if (valread <= 0)
                {
                    close(sd);
                    clients[i].sock = 0;
                    clients[i].topic_count = 0;
                    printf("Client disconnected\n");
                }
                else
                {
                    buffer[valread] = '\0';
                    buffer[strcspn(buffer, "\r\n")] = 0;

                    // parse command
                    if (strncmp(buffer, "SUB ", 4) == 0)
                    {
                        char topic[50];
                        sscanf(buffer + 4, "%s", topic);
                        subscribe(&clients[i], topic);

                        char msg[] = "Subscribed!\n";
                        send(sd, msg, strlen(msg), 0);
                    }
                    else if (strncmp(buffer, "UNSUB ", 6) == 0)
                    {
                        char topic[50];
                        sscanf(buffer + 6, "%s", topic);

                        if (is_subscribed(&clients[i], topic))
                        {
                            unsubscribe(&clients[i], topic);
                            char msg[] = "Unsubscribed!\n";
                            send(sd, msg, strlen(msg), 0);
                        }
                        else
                        {
                            char msg[] = "Not subscribed to this topic!\n";
                            send(sd, msg, strlen(msg), 0);
                        }
                    }
                    else if (strncmp(buffer, "PUB ", 4) == 0)
                    {
                        char topic[50], msg[BUFFER_SIZE];

                        sscanf(buffer + 4, "%s %[^\n]", topic, msg);

                        publish(topic, msg);
                    }
                }
            }
        }
    }

    return 0;
}