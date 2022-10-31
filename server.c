#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define LOG(fmt, ...) printf(fmt" %s:%d\n", ##__VA_ARGS__, __FILENAME__, __LINE__)
#define EXIT(error) do {perror(error); exit(EXIT_FAILURE);} while(0)

#define MAX_REQUEST_LEN 10240
#define MAX_METHOD_LEN  32
#define MAX_URI_LEN     256

char buf[25600];

char header[] = "HTTP/1.0 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "\r\n";

char unimplement_header[] = "HTTP/1.0 501 Method Not Implemented\r\n"
                            "Content-Type: text/html\r\n"
                            "\r\n"
                            "Method Not Implemented";

char not_found_header[] = "HTTP/1.0 404 NOT FOUND\r\n"
                            "Content-Type: text/html\r\n"
                            "\r\n"
                            "The resource specified is unavailable.\r\n";


int create_server_fd (unsigned int port) {
    int serverfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverfd == -1) {
        printf("socket failed\n");
        return -1;
    }
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverfd,(struct sockaddr *)&server, sizeof(server)) == -1) {
        printf("bind failed\n");
        return -2;
    }
    if (listen(serverfd, 10) == -1) {
        printf("listen failed\n");
        return -3;
    }

    return serverfd;
}

int main() {
    int serverfd, connfd;
    pthread_t tid;
    struct sockaddr_in client;
    socklen_t clientlen = sizeof(client);
    int port = 5000;

    serverfd = create_server_fd(port);
    LOG("Server started, listen port %d", port);
    while (1) {
        connfd = accept(serverfd, (struct sockaddr *)&client, &clientlen);
        int recv_len = recv(connfd, buf, sizeof(buf), 0);
        if (recv_len <= 0) {
            printf("recv failed\n");
            break;
        }
        if (strstr(buf, "GET")) {
            printf("get\n");
            FILE *f = fopen("profile.html", "r");
            if (!f) {
                printf("cant find html\n");
                send(connfd, not_found_header, sizeof(not_found_header), 0);
                break;
            }
            send(connfd, header, sizeof(header), 0);
            if (f)
            while (fgets(buf, sizeof(buf), f) != NULL) {
                send(connfd, buf, strlen(buf), 0);
            }
            send(connfd, "\r\n", 2, 0);
            fclose(f);
        }
        else {
            printf("not implement\n");
            send(connfd,  unimplement_header, sizeof(unimplement_header), 0);
            break;
        }

    }
    return 0;
}
