#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static int stop = 0;

static void handle_term(int sig)
{
    stop = 1;
}

int main(int argc, char *argv[]) 
{
    signal(SIGTERM, handle_term);

    if (argc < 3) {
        printf("usage: %s ip_address port\n", argv[0]);
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
    socklen_t client_socklen = sizeof (client);
    int confd = accept(sock, (struct sockaddr *)&client, &client_socklen);

    if (confd < 0) {
        printf("errno is: %d\n", errno);
    } else {
        char remote[INET_ADDRSTRLEN];
        printf("connected with ip: %s port: %d\n", inet_ntop(AF_INET, &client.sin_addr, 
                    remote, INET_ADDRSTRLEN), ntohs(client.sin_port));
        close(confd);
    }

    close(sock);
    return 0;
}
