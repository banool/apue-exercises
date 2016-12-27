#include "apue.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
//-#include "setfl.c"
//-#include "error.c"

int main(int argc, char *argv[]) {
    int fds[2];

    if (pipe(fds) < 0) {
        perror("pipe error");
        return -1;
    }

    set_fl(fds[1], O_NONBLOCK);

    int ret;
    int counter = 0;
    while ((ret = write(fds[1], "writing data friend", 19)) > 0) {
        counter += ret;
        // This fun little escape makes it reprint on the same line.
        printf("\033[A\33[2K\r%5d bytes written\n", counter);
    }

    printf("write failed with error %d: %s\n", errno, strerror(errno));
    return 0;
}
