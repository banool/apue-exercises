#include "apue.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#if defined(BSD) || defined(MACOS) || defined(SOLARIS)
#include <sys/filio.h>
#endif

int enable_async_io(int sockfd) {
    if (fcntl(sockfd, F_SETOWN, getpid()) == -1) 
        return -1;
    int n = 1; // We want to set this behaviour on.
    if (ioctl(sockfd, FIOASYNC, &n) == -1)
        return -1;
    return 0;
}

int disable_async_io(int sockfd) {
    // Doesn't seem like we can undo the socket ownership?

    int n = 0; // We want to set this behaviour on.
    if (ioctl(sockfd, FIOASYNC, &n) == -1)
        return -1;
    return 0;
}

int main() {
    return 0;
}
