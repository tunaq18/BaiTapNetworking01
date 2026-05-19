#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 13
#define BUFFER_SIZE 1024

typedef struct
{
    int fd;
    char name[50];
    int is_registered;
} Client;

Client clients[MAX_CLIENTS];

pthread_mutex_t lock;

void broadcast(int sender_idx, char *message)
{
    char time_str[50];

    time_t now = time(NULL);

    struct tm *t = localtime(&now);

    strftime(time_str,
             sizeof(time_str),
             "%Y/%m/%d %H:%M:%S",
             t);

    char final_msg[1200];

    sprintf(final_msg,
            "%s %s: %s\n",
            time_str,
            clients[sender_idx].name,
            message);

    pthread_mutex_lock(&lock);

    for(int i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i].fd!=-1 &&
           i!=sender_idx &&
           clients[i].is_registered)
        {
            send(clients[i].fd,
                 final_msg,
                 strlen(final_msg),
                 0);
        }
    }

    pthread_mutex_unlock(&lock);
}

void* handleClient(void *arg)
{
    int index=*(int*)arg;

    free(arg);

    char buffer[BUFFER_SIZE];

    int sd=clients[index].fd;

    while(1)
    {
        memset(buffer,0,sizeof(buffer));

        int n=recv(sd,
                   buffer,
                   BUFFER_SIZE,
                   0);

        if(n<=0)
        {
            printf("%s ngat ket noi\n",
                    clients[index].name);

            close(sd);

            pthread_mutex_lock(&lock);

            clients[index].fd=-1;
            clients[index].is_registered=0;

            pthread_mutex_unlock(&lock);

            break;
        }

        buffer[strcspn(buffer,"\r\n")]=0;

        if(!clients[index].is_registered)
        {
            char name[50];

            if(sscanf(buffer,
                      "client_id: %s",
                      name)==1)
            {
                strcpy(clients[index].name,
                       name);

                clients[index].is_registered=1;

                send(sd,
                     "Dang ky thanh cong!\n",
                     23,
                     0);

                printf("%s da tham gia\n",
                        clients[index].name);
            }
            else
            {
                send(sd,
                     "Sai cu phap! client_id: ten\n",
                     33,
                     0);
            }
        }
        else
        {
            broadcast(index,buffer);
        }
    }

    pthread_exit(NULL);
}

int main()
{
    int serverSock,newSock;

    struct sockaddr_in serverAddr;

    for(int i=0;i<MAX_CLIENTS;i++)
    {
        clients[i].fd=-1;
        clients[i].is_registered=0;
    }

    pthread_mutex_init(&lock,NULL);

    serverSock=
    socket(AF_INET,SOCK_STREAM,0);

    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(PORT);
    serverAddr.sin_addr.s_addr=INADDR_ANY;

    bind(serverSock,
         (struct sockaddr*)&serverAddr,
         sizeof(serverAddr));

    listen(serverSock,5);

    printf("Server dang chay...\n");

    while(1)
    {
        newSock=
        accept(serverSock,NULL,NULL);

        printf("Client moi ket noi\n");

        pthread_mutex_lock(&lock);

        for(int i=0;i<MAX_CLIENTS;i++)
        {
            if(clients[i].fd==-1)
            {
                clients[i].fd=newSock;

                send(newSock,
                     "Nhap: client_id: ten\n",
                     25,
                     0);

                int *index=
                malloc(sizeof(int));

                *index=i;

                pthread_t t;

                pthread_create(
                    &t,
                    NULL,
                    handleClient,
                    index);

                pthread_detach(t);

                break;
            }
        }

        pthread_mutex_unlock(&lock);
    }

    close(serverSock);

    return 0;
}