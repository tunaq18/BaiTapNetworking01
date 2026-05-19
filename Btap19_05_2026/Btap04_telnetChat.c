#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUF_SIZE 1024

pthread_mutex_t lock;


int check_login(char *user, char *pass)
{
    FILE *f = fopen("users.txt", "r");

    if(f == NULL)
        return 0;

    char u[100], p[100];

    while(fscanf(f, "%s %s", u, p) != EOF)
    {
        if(strcmp(user, u) == 0 &&
           strcmp(pass, p) == 0)
        {
            fclose(f);
            return 1;
        }
    }

    fclose(f);

    return 0;
}


void* handle_client(void *arg)
{
    int clientSock = *(int*)arg;

    free(arg);

    char buf[BUF_SIZE];
    char user[100];
    char pass[100];

    while(1)
    {
        send(clientSock, "Username: ", 10, 0);

        int n = recv(clientSock, user, sizeof(user)-1, 0);

        if(n <= 0)
            break;

        user[n] = 0;

        send(clientSock, "Password: ", 10, 0);

        n = recv(clientSock, pass, sizeof(pass)-1, 0);

        if(n <= 0)
            break;

        pass[n] = 0;

        user[strcspn(user,"\r\n")] = 0;
        pass[strcspn(pass,"\r\n")] = 0;

        if(check_login(user, pass))
        {
            send(clientSock, "Login success\n", 14, 0);
            break;
        }

        send(clientSock, "Login failed\n", 13, 0);
    }


    while(1)
    {
        send(clientSock, "cmd> ", 5, 0);

        int n = recv(clientSock, buf, sizeof(buf)-1, 0);

        if(n <= 0)
            break;

        buf[n] = 0;

        buf[strcspn(buf,"\r\n")] = 0;

        if(strcmp(buf, "exit") == 0)
            break;

        char command[BUF_SIZE];

        sprintf(command, "%s > out.txt", buf);

        system(command);

        pthread_mutex_lock(&lock);

        FILE *f = fopen("out.txt", "r");

        if(f != NULL)
        {
            while(fgets(buf, sizeof(buf), f))
            {
                send(clientSock, buf, strlen(buf), 0);
            }

            fclose(f);

            remove("out.txt");
        }

        pthread_mutex_unlock(&lock);
    }

    close(clientSock);

    pthread_exit(NULL);
}


int main()
{
    int serverSock;

    struct sockaddr_in server;

    pthread_mutex_init(&lock, NULL);

    serverSock = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(serverSock, (struct sockaddr*)&server, sizeof(server));

    listen(serverSock, 5);

    printf("Server running on port %d...\n", PORT);


    while(1)
    {
        int clientSock = accept(serverSock, NULL, NULL);

        printf("Client connected\n");

        int *p = malloc(sizeof(int));

        *p = clientSock;

        pthread_t t;

        pthread_create(&t, NULL, handle_client, p);

        pthread_detach(t);
    }

    close(serverSock);

    return 0;
}