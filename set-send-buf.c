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
        printf("usage: %s server_addr port send_buf_size\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    /* set and get print tcp send buffer size. */
    int send_buf_size = 0;
    int opt_len = sizeof (send_buf_size);
    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &send_buf_size, (socklen_t *)&opt_len);
    printf("default send buffer size is %d\n", send_buf_size);

    send_buf_size = atoi(argv[3]);
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &send_buf_size, sizeof (send_buf_size));
    printf("set send buffer size %d\n", send_buf_size);

    getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &send_buf_size, (socklen_t *)&opt_len);
    printf("get send buffer size is %d\n", send_buf_size);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof (server_addr)) == -1) {
        perror("connection server failed");
    } else {
        char buf[BUFFER_SIZE];
        while (!stop) {
           scanf("%s", buf); 
           send(sock, buf, strlen(buf), 0);
        }
        close(sock);
    }
    return 0;
}
