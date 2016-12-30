#include "apue.h"
#include <sys/types.h> // For portability
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
//-#include "../sockets/initsrv2.c"
//-#include "error.c"
//-#include "stat_info.c"

#define QLEN 10

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 256
#endif

extern int initserver(int, const struct sockaddr *, socklen_t, int);
extern void print_stat_info(struct stat statbuf);

int main(int argc, char *argv[]) {

    struct addrinfo *ailist, *aip;
    struct addrinfo hint;
    int             sockfd, err, n;
    char            *host;

    if ((n = sysconf(_SC_HOST_NAME_MAX)) < 0)
        n = HOST_NAME_MAX;  /* best guess */
    if ((host = malloc(n)) == NULL)
        err_sys("malloc error");
    if (gethostname(host, n) < 0)
        err_sys("gethostname error");
    memset(&hint, 0, sizeof(hint));
    hint.ai_flags = AI_CANONNAME;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_canonname = NULL;
    hint.ai_addr = NULL;
    hint.ai_next = NULL;
    if ((err = getaddrinfo(host, "ftp", &hint, &ailist)) != 0) {
        perror(gai_strerror(err));
        return -1;
    }
    for (aip = ailist; aip != NULL; aip = aip->ai_next) {
        if ((sockfd = initserver(SOCK_STREAM, aip->ai_addr,
          aip->ai_addrlen, QLEN)) < 0) {
            perror("initserver error");
            return -1;
        }
    }

    struct stat statbuf;
    if (fstat(sockfd, &statbuf) < 0) {
        perror("fstat error");
        return -1;
    }
    print_stat_info(statbuf);
}
