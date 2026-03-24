#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int server_fd,client;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];
    // Tạo socket
    server_fd = socket(AF_INET,SOCK_STREAM,0);

    // Thiết lập địa chỉ server
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // Bind socket với địa chỉ
    bind(server_fd,(struct sockaddr*)&server,sizeof(server));
    // Lắng nghe kết nối
    listen(server_fd,5);
    printf("Server is listening on port %d...\n",PORT);
    while(1){
        //Chấp nhận kết nối từ client
        client = accept(server_fd,NULL,NULL);
        printf("Client connected.\n");
        
        int count = 0;
        char full_data[8192] = "";
        
        while(1){
            // Nhận dữ liệu từ client
            int bytes_received = recv(client,buffer,sizeof(buffer)-1,0);
            
            if(bytes_received > 0){
                buffer[bytes_received] = '\0';
                strcat(full_data,buffer);
            }
            else if(bytes_received == 0){
                // Client đóng kết nối
                break;
            }
            else{
                perror("recv");
                break;
            }
        }

        char normalized_data[8192];
        int j = 0;
        for(int i = 0; full_data[i] != '\0' && j < (int)sizeof(normalized_data) - 1; i++){
            if(full_data[i] != '\n'){
                normalized_data[j++] = full_data[i];
            }
        }
        normalized_data[j] = '\0';
        
        // Đếm số lần xuất hiện chuỗi "0123456789"
        char *pos = normalized_data;
        while((pos = strstr(pos,"0123456789")) != NULL){
            count++;
            pos++;
        }
        
        printf("Number of '0123456789' occurrences: %d\n",count);
        printf("Full text received:\n%s\n",full_data);
        
        close(client);
    }
    
    close(server_fd);
    return 0;
}