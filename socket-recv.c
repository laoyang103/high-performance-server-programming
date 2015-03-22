#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <libgen.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static int stop = 0;
#define BUFFER_SIZE 1024

static void handle_term(int sig)
{
    stop = 1;
}

int main(int argc, char *argv[]) 
{
    signal(SIGTERM, handle_term);

    if (argc < 3) {
        printf("usage: %s ip_address port\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    struct sockaddr_in addr;
    bzero(&addr, sizeof (addr));
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &addr.sin_addr);
    addr.sin_port = htons(port);

    int ret = bind(sock, (struct sockaddr *)&addr, sizeof (addr));
    assert(ret != -1);

    ret = listen(sock, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t client_addrlen = sizeof (client);

    int connfd = accept(sock, (struct sockaddr *)&client, &client_addrlen);
    if (connfd < 0) {
        perror("accept connection failed");
    } else {
        int recv_len;
        char buf[BUFFER_SIZE];
        while (!stop) {
            recv_len = recv(connfd, buf, BUFFER_SIZE - 1, 0);
            if (recv_len <= 0) break;
            buf[recv_len] = 0;
            printf("%d: %s\n", recv_len, buf);
        }
        close(connfd);
    }

    close(sock);
    return 0;
}
