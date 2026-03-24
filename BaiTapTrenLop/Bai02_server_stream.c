#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int count_substring(char *data, char *pattern){
    int count = 0;
    int len = strlen(pattern);
    for(int i=0;data[i];i++){
        if(strncmp(&data[i],pattern,len)==0){
            count++;
        }
    }
    return count;
}

int main(){
    int server_fd,client_fd;
    struct sockaddr_in addr;
    char buffer[BUFFER_SIZE];
    char remain[20]="";
    char pattern[] = "0123456789";
    // Tạo socket
    server_fd = socket(AF_INET,SOCK_STREAM,0);

    // Thiết lập địa chỉ server
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    // Bind socket với địa chỉ
    bind(server_fd,(struct sockaddr*)&addr,sizeof(addr));
    // Lắng nghe kết nối
    listen(server_fd,5);
    printf("Server is listening on port %d...\n",PORT);
    //Chấp nhận kết nối từ client
    client_fd = accept(server_fd,NULL,NULL);
    printf("Client connected.\n");
    int total=0;
    while(1){
        int n =recv(client_fd,buffer,sizeof(buffer)-1,0);
        if(n<=0) break;
        buffer[n]='\0';
        
        char combine[BUFFER_SIZE+20];
        strcpy(combine,remain);
        strcat(combine,buffer);
        
        int count = count_substring(combine,pattern);
        total+=count;
        printf("So lan xuat hien: %d\n",total);
        int len = strlen(combine);
        if(len >=9){
            strncpy(remain,combine+len-9,9);
            remain[9]= '\0';
        }else{
            strcpy(remain,combine);
        }
    }
    close(client_fd);
    close(server_fd);
    return 0;
}