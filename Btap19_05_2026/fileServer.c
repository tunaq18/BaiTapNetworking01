#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <arpa/inet.h>

int main()
{
    int serverSock, clientSock;

    serverSock = socket(AF_INET, SOCK_STREAM,0);

    // bind
    // listen

    while(1)
    {
        clientSock = accept(serverSock,NULL,NULL);

        int pid = fork();

        if(pid == 0)
        {
            // tiến trình con

            close(serverSock);

            printf("Child %d xu ly client\n",getpid());

            // gửi danh sách file
            // nhận tên file
            // gửi nội dung file

            close(clientSock);

            exit(0);
        }
        else
        {
            // tiến trình cha

            close(clientSock);

            // dọn zombie
            waitpid(-1,NULL,WNOHANG);
        }
    }

    close(serverSock);

    return 0;
}