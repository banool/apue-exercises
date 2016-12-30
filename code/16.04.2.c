#include "apue.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 256

int main(int argc, char *argv[]) {
    // the "in" in sockaddr_in is for internet, not in(coming).
    struct sockaddr_in serv_addr;
    struct in_addr address;
    int sfd;
    char buf[BUFSIZE];

    if (argc != 3) {
        printf("Usage: %s <address> <port>\n", argv[0]);
        return 1;
    }
    serv_addr.sin_family = AF_INET;
    if (inet_aton(argv[1], &address) != 1) {
        fprintf(stderr, "Invalid address format. Use ipv4 dot notation.\n");
        return -1;
    }
    serv_addr.sin_addr.s_addr = address.s_addr;
    serv_addr.sin_port = htons(atoi(argv[2]));

    if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        err_sys("socket error");
    }

    if (connect(sfd, &serv_addr, sizeof(serv_addr)) == -1) {
        err_sys("connect error");
    }

    if (recv(sfd, buf, BUFSIZE, 0) < 0) {
        err_sys("recv error");
    }

    close(sfd);

    // The incoming string includes a newline, see fgets.
    printf("Number of processes running on server: %s", buf);

    return 0;
}
