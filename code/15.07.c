#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int
main(void)
{
    int fds[2];
    pid_t pid;
    fd_set rfds;
    struct timeval tv;
    int retval;

    // 0 is read end, 1 is write end.
    pipe(fds);

    /* Watch stdin (fd 0) to see when it has input. */
    FD_ZERO(&rfds);
    FD_SET(fds[0], &rfds);

    if ((pid = fork()) < 0) {
        perror("fork err");
        return -1;
    } else if (pid == 0) { // Child
        // After 5 seconds the parent, which is looping select, will see
        // that the pipe has been closed.
        sleep(5);
        close(fds[1]);
        printf("Closed pipe write end\n");
        exit(0);
    }

    /* Wait up to one second. */
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while (1) {
        write(fds[1], "hey\n", 4);
        // The first arg is the highest descriptor to check up to.
        retval = select(fds[0]+1, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now! */

        if (retval == -1)
            perror("select()");
        else if (retval) {
            printf("Data is available now.\n");
            /* FD_ISSET(0, &rfds) will be true. */
        }
        else {
            printf("%d No data within one second.\n", retval);
            tv.tv_sec = 1;
            tv.tv_usec = 0;
        }
    }
    printf("blah\n");
    waitpid(pid, NULL, 0);

    exit(EXIT_SUCCESS);
}
