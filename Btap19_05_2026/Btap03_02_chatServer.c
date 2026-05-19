#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 9000
#define SIZE 1024

typedef struct
{
    int from;
    int to;
} PAIR;

int queue[100];
int count = 0;

pthread_mutex_t lock;

void* chat(void *arg)
{
    PAIR *p = (PAIR*)arg;

    char msg[SIZE];
    int n;

    while(1)
    {
        memset(msg, 0, SIZE);

        n = recv(p->from, msg, SIZE, 0);

        if(n <= 0)
        {
            send(p->to, "Partner disconnected\n", 21, 0);

            close(p->from);
            close(p->to);

            break;
        }

        send(p->to, msg, strlen(msg), 0);
    }

    free(p);

    pthread_exit(NULL);
}

int main()
{
    int serverSock, clientSock;

    struct sockaddr_in serverAddr;

    serverSock = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSock,
          (struct sockaddr*)&serverAddr,
          sizeof(serverAddr));

    listen(serverSock, 5);

    pthread_mutex_init(&lock, NULL);

    printf("Server running...\n");

    while(1)
    {
        clientSock = accept(serverSock, NULL, NULL);

        pthread_mutex_lock(&lock);

        queue[count++] = clientSock;

        printf("Client joined\n");

        if(count >= 2)
        {
            int c1 = queue[0];
            int c2 = queue[1];

            count = 0;

            send(c1, "Matched!\n", 10, 0);
            send(c2, "Matched!\n", 10, 0);

            pthread_t t1, t2;

            PAIR *p1 = malloc(sizeof(PAIR));
            PAIR *p2 = malloc(sizeof(PAIR));

            p1->from = c1;
            p1->to = c2;

            p2->from = c2;
            p2->to = c1;

            pthread_create(&t1, NULL, chat, p1);
            pthread_create(&t2, NULL, chat, p2);

            pthread_detach(t1);
            pthread_detach(t2);
        }

        pthread_mutex_unlock(&lock);
    }

    close(serverSock);

    return 0;
}