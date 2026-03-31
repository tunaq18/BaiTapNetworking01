/*******************************************************************************
* @file    udp_chat.c
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s port_s ip_d port_d\n", argv[0]);
        return 1;
    }

    int port_s = atoi(argv[1]);
    char *ip_d = argv[2];
    int port_d = atoi(argv[3]);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket() failed");
        return 1;
    }

    // bind local port
    struct sockaddr_in local_addr = {0};
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = INADDR_ANY;
    local_addr.sin_port = htons(port_s);

    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        perror("bind() failed");
        close(sock);
        return 1;
    }

    // địa chỉ đích
    struct sockaddr_in dest_addr = {0};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port_d);
    inet_pton(AF_INET, ip_d, &dest_addr.sin_addr);

    // set non-blocking socket
    unsigned long ul = 1;
    ioctl(sock, FIONBIO, &ul);

    // set non-blocking stdin
    ioctl(0, FIONBIO, &ul);

    printf("UDP chat started (%d -> %s:%d)\n", port_s, ip_d, port_d);

    char buf[BUF_SIZE];

    while (1) {
        // Nhận
        struct sockaddr_in from_addr;
        socklen_t from_len = sizeof(from_addr);

        int len = recvfrom(sock, buf, sizeof(buf) - 1, 0,
                           (struct sockaddr *)&from_addr, &from_len);

        if (len > 0) {
            buf[len] = 0;
            printf("\n[RECV]: %s\n", buf);
            fflush(stdout);
        } 
        else if (len < 0 && errno != EWOULDBLOCK) {
            perror("recvfrom error");
        }

        // Gửi
        int input_len = read(0, buf, sizeof(buf) - 1);

        if (input_len > 0) {
            buf[input_len] = 0;

            sendto(sock, buf, strlen(buf), 0,
                   (struct sockaddr *)&dest_addr, sizeof(dest_addr));

            printf("[SENT]: %s\n", buf);
        } 
        else if (input_len < 0 && errno != EWOULDBLOCK) {
            perror("stdin error");
        }

    }

    close(sock);
    return 0;
}