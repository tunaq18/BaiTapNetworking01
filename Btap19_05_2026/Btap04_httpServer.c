#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define THREAD_POOL_SIZE 5

int listener;

pthread_mutex_t lock;


void* worker(void *arg)
{
    while(1)
    {
        pthread_mutex_lock(&lock);

        int client = accept(listener, NULL, NULL);

        pthread_mutex_unlock(&lock);

        if(client < 0)
            continue;

        printf("Thread %ld handling client %d\n",
               pthread_self(),
               client);

        char buf[1024];

        int ret = recv(client, buf, sizeof(buf)-1, 0);

        if(ret > 0)
        {
            buf[ret] = 0;

            printf("Received:\n%s\n", buf);
        }

        char *msg =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html><body><h1>Xin chao cac ban</h1></body></html>";

        send(client, msg, strlen(msg), 0);

        close(client);

        printf("Client done by thread %ld\n",
               pthread_self());
    }

    pthread_exit(NULL);
}


int main()
{
    listener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(listener, (struct sockaddr*)&server, sizeof(server));

    listen(listener, 5);

    pthread_mutex_init(&lock, NULL);

    pthread_t threads[THREAD_POOL_SIZE];

    for(int i=0;i<THREAD_POOL_SIZE;i++)
    {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    printf("Prethread server running on port %d...\n", PORT);

    for(int i=0;i<THREAD_POOL_SIZE;i++)
    {
        pthread_join(threads[i], NULL);
    }

    close(listener);

    return 0;
}