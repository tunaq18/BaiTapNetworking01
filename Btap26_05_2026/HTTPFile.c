#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <sys/stat.h>

#define PORT 8080
#define BUFFER_SIZE 4096

char* get_content_type(char *path) {

    char *ext = strrchr(path, '.');

    if (ext == NULL)
        return "text/plain";

    if (!strcmp(ext, ".html"))
        return "text/html";

    if (!strcmp(ext, ".txt"))
        return "text/plain";

    if (!strcmp(ext, ".jpg"))
        return "image/jpeg";

    if (!strcmp(ext, ".jpeg"))
        return "image/jpeg";

    if (!strcmp(ext, ".png"))
        return "image/png";

    if (!strcmp(ext, ".gif"))
        return "image/gif";

    if (!strcmp(ext, ".mp3"))
        return "audio/mpeg";

    if (!strcmp(ext, ".mp4"))
        return "video/mp4";

    return "application/octet-stream";
}


void send_directory(int client, char *path) {

    DIR *dir = opendir(path);

    if (dir == NULL)
        return;

    char body[50000];

    strcpy(body,
    "<html>"
    "<head><title>File Server</title></head>"
    "<body>"
    "<h1>Directory Listing</h1>"
    "<ul>");

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL) {

        if (!strcmp(entry->d_name, "."))
            continue;

        if (!strcmp(entry->d_name, ".."))
            continue;

        char fullpath[1024];

        sprintf(fullpath, "%s/%s", path, entry->d_name);

        struct stat st;

        stat(fullpath, &st);

        strcat(body, "<li>");

        char temp[2000];

        if (S_ISDIR(st.st_mode)) {

            sprintf(temp,
                    "<b><a href=\"%s\">%s</a></b>",
                    fullpath + 2,
                    entry->d_name);

        } else {

            sprintf(temp,
                    "<i><a href=\"%s\">%s</a></i>",
                    fullpath + 2,
                    entry->d_name);
        }

        strcat(body, temp);

        strcat(body, "</li>");
    }

    strcat(body,
    "</ul>"
    "</body>"
    "</html>");

    char header[1024];

    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            strlen(body));

    send(client, header, strlen(header), 0);

    send(client, body, strlen(body), 0);

    closedir(dir);
}


void send_file(int client, char *path) {

    FILE *f = fopen(path, "rb");

    if (f == NULL)
        return;

    fseek(f, 0, SEEK_END);

    long size = ftell(f);

    rewind(f);

    char *buffer = malloc(size);

    fread(buffer, 1, size, f);

    char header[1024];

    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n"
            "\r\n",
            get_content_type(path),
            size);

    send(client, header, strlen(header), 0);

    send(client, buffer, size, 0);

    free(buffer);

    fclose(f);
}


int main() {

    int server = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));

    listen(server, 5);

    printf("File server running at port %d...\n", PORT);

    while (1) {

        int client = accept(server, NULL, NULL);

        char request[BUFFER_SIZE];

        int len = recv(client, request, sizeof(request) - 1, 0);

        if (len <= 0) {

            close(client);

            continue;
        }

        request[len] = '\0';

        printf("\nREQUEST:\n%s\n", request);

        char path[1024] = "";

        char *start = strstr(request, "GET /");

        if (start != NULL) {

            start += 5;

            char *end = strstr(start, " HTTP");

            if (end != NULL) {

                int n = end - start;

                strncpy(path, start, n);

                path[n] = '\0';
            }
        }

        if (strlen(path) == 0) {

            strcpy(path, ".");

        } else {

            char temp[1024];

            sprintf(temp, "./%s", path);

            strcpy(path, temp);
        }

        printf("PATH = %s\n", path);

        struct stat st;

        if (stat(path, &st) == 0) {

            if (S_ISDIR(st.st_mode))
                send_directory(client, path);

            else
                send_file(client, path);

        } else {

            char *msg =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type:text/html\r\n\r\n"
            "<h1>404 File Not Found</h1>";

            send(client, msg, strlen(msg), 0);
        }

        close(client);
    }

    close(server);

    return 0;
}