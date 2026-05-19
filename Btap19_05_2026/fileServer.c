#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>

#define PORT 9000
#define SIZE 1024
#define FOLDER "./files"

void sendFileList(int client)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(FOLDER);

    if(dir==NULL)
    {
        send(client,
              "ERROR No files to download\r\n",
              30,0);

        close(client);
        exit(0);
    }

    int count=0;

    while((entry=readdir(dir))!=NULL)
    {
        if(entry->d_type==DT_REG)
            count++;
    }

    if(count==0)
    {
        send(client,
              "ERROR No files to download\r\n",
              30,0);

        closedir(dir);

        close(client);

        exit(0);
    }

    rewinddir(dir);

    char msg[4096]="";

    sprintf(msg,"OK %d\r\n",count);

    while((entry=readdir(dir))!=NULL)
    {
        if(entry->d_type==DT_REG)
        {
            strcat(msg,entry->d_name);
            strcat(msg,"\r\n");
        }
    }

    strcat(msg,"\r\n");

    send(client,msg,strlen(msg),0);

    closedir(dir);
}

void sendFile(int client)
{
    char filename[100];

    while(1)
    {
        memset(filename,0,sizeof(filename));

        int n=recv(client,
                   filename,
                   sizeof(filename),
                   0);

        if(n<=0)
            return;

        filename[strcspn(filename,"\r\n")]=0;

        char path[200];

        sprintf(path,
                "%s/%s",
                FOLDER,
                filename);

        int fd=open(path,O_RDONLY);

        if(fd<0)
        {
            send(client,
                 "ERROR File not found\r\n",
                 24,
                 0);

            continue;
        }

        struct stat st;

        stat(path,&st);

        int filesize=st.st_size;

        char header[100];

        sprintf(header,
                "OK %d\r\n",
                filesize);

        send(client,
             header,
             strlen(header),
             0);

        char buffer[SIZE];

        while((n=read(fd,
                      buffer,
                      SIZE))>0)
        {
            send(client,
                 buffer,
                 n,
                 0);
        }

        close(fd);

        break;
    }
}

int main()
{
    int serverSocket;
    int clientSocket;

    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    socklen_t len;

    serverSocket=socket(
                    AF_INET,
                    SOCK_STREAM,
                    0);

    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port=htons(PORT);
    serverAddr.sin_addr.s_addr=INADDR_ANY;

    bind(serverSocket,
         (struct sockaddr*)&serverAddr,
         sizeof(serverAddr));

    listen(serverSocket,5);

    printf("Server started...\n");

    while(1)
    {
        len=sizeof(clientAddr);

        clientSocket=
        accept(serverSocket,
              (struct sockaddr*)&clientAddr,
              &len);

        int pid=fork();

        if(pid==0)
        {
            close(serverSocket);

            sendFileList(clientSocket);

            sendFile(clientSocket);

            close(clientSocket);

            exit(0);
        }
        else
        {
            close(clientSocket);

            waitpid(
                -1,
                NULL,
                WNOHANG);
        }
    }

    close(serverSocket);

    return 0;
}