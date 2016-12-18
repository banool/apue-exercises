#include <stdio.h>
#include <unistd.h>

int main() {
    dup2(1, 5);
    // Check out the open fds with lsof.
    while(1) {
        sleep(1);
    }
    return 0;
}

int dup2(int fd1, int fd2) {

    if (fd1 == fd2)
        return fd2;

    close(fd2);

    // TODO how to duplicate without dup or fnctl?
    // Just opening fd2 will open a new file descriptor no? It won't point to
    // the same file table entry as fd1 yeah?

    // How about this: Keep duplicating until we get the fd we wanted, store
    // the file descriptors it returned in an array (so we don't close any fds
    // that were already open), and then close all the fds we opened on the way
    // to fd2.

    // We make an array of fds with the number of slots corresponding
    // to fd2. This leaves us enough space for the fds we create with
    // dup all the way up to fd2, though we likely won't need all of
    // the slots, since some fds will be in use (stdin/out/err).
    int fds[fd2];
    int num_made = 0;
    int descriptor = -1;
    // Duplicate descriptors up until we get fd2.
    while (descriptor != fd2) {
        descriptor = dup(fd1);
        fds[num_made] = descriptor;
        num_made++;
    }

    // Close all the fds we opened up until fd2 (hence the -1).
    for (int i = 0; i < num_made - 1; i++) {
        close(fds[i]);
    }

    return descriptor;
}
