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
        printf("usage: %s server_addr server_port local_port\n", basename(argv[0]));
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    int loc_port = atoi(argv[3]);

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    int reuse = 1;
    int loc_sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(loc_sock >= 0);

    struct sockaddr_in loc_addr;
    bzero(&loc_addr, sizeof (loc_addr));
    loc_addr.sin_family = AF_INET;
    loc_addr.sin_addr.s_addr = htons(INADDR_ANY);
    loc_addr.sin_port = htons(loc_port);

    /* It will failed when loc_addr is TIME_WAIT, unless set SO_REUSEADDR opt. */
    setsockopt(loc_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof (reuse));
    if (-1 == bind(loc_sock, (struct sockaddr *)&loc_addr, sizeof (loc_addr))) {
        perror("bind address failed");
        exit(1);
    }

    if (connect(loc_sock, (struct sockaddr *)&server_addr, sizeof (server_addr)) == -1) {
        perror("connection server failed");
    } else {
        char buf[BUFFER_SIZE];
        while (!stop) {
           scanf("%s", buf); 
           send(loc_sock, buf, strlen(buf), 0);
        }
        close(loc_sock);
    }
    return 0;
}
