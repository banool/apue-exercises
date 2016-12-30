#include "apue.h"
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//-#include "../sockets/initsrv2.c"

#define MYPORT 4321
#define QUEUE_LEN 10
#define BUFLEN 256

extern int initserver(int, const struct sockaddr *, socklen_t, int);
int get_num_proc();

int main(int argc, char *argv[]) {
    struct sockaddr_in myaddr, client;
    int sfd, clientsfd;
    char buf[BUFLEN];

    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = INADDR_ANY;
    myaddr.sin_port = htons (MYPORT); 
    // See the answer here about INADDR_ANY, about finding where the client
    // is connected from: https://stackoverflow.com/questions/4046616/sockets-how-to-find-out-what-port-and-address-im-assigned

    if ((sfd = initserver(SOCK_STREAM, (struct sockaddr*) &myaddr, 
      sizeof(myaddr), QUEUE_LEN)) < 0) {
        perror("initserver error");
        return -1;
    }

    // Block waiting for a client connection.
    socklen_t clen = (socklen_t)sizeof(client);
    while ((clientsfd = accept(sfd, (struct sockaddr*) &client, 
      &clen)) != -1) {
        // We don't worry with spawning a thread/process to do this task
        // because it's so quick.
        if (get_num_proc(&buf, BUFLEN) == -1) {
            perror("get_num_proc error");
            return -1;
        }
        // +1 so we send the null byte too.
        if (send(clientsfd, buf, strlen(buf)+1, 0) != strlen(buf)+1) {
            perror("send error");
            return -1;
        }
    }
    return 0;
}

// Stores the number of processes as a string in buf.
// Originally this returned an integer number, but we have to send it
// back as a stream of chars anyway so no point converting it twice.
int get_num_proc(char *buf, int len) {
    FILE *fp;

    if ((fp = popen("ps aux | wc -l", "r")) == NULL) {
        return -1;
    }

    // fgets appends null byte.
    if (fgets(buf, len, fp))
        if (ferror(fp))
            return -1;
    /*
    errno = 0;
    res = strtol(buf, NULL, 10);
    if (errno != 0)
        return -1;
    */
    pclose(fp);
    return 0;
}
