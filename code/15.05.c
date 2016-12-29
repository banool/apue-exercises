#include "apue.h"
//-#include "error.c"

static void sig_pipe(int);      /* our signal handler */

int
main(void)
{
    int     n, fd1[2], fd2[2];
    FILE    *f_in, *f_out;
    pid_t   pid;
    char    line[MAXLINE];

    if (signal(SIGPIPE, sig_pipe) == SIG_ERR)
        err_sys("signal error");

    if (pipe(fd1) < 0 || pipe(fd2) < 0)
        err_sys("pipe error");

    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else if (pid > 0) {                           /* parent */
        close(fd1[0]);
        close(fd2[1]);
        f_in  = fdopen(fd2[0], "r"); // error checking, who needs it?
        f_out = fdopen(fd1[1], "w");
        // NULL means let it do the buffer allocating. Line buffering of course.
        setvbuf(f_in, NULL, _IOLBF, 0);
        setvbuf(f_out, NULL, _IOLBF, 0);

        while (fgets(line, MAXLINE, stdin) != NULL) {
            n = strlen(line);
            if (fputs(line, f_out) == EOF)
                err_sys("fputs error to pipe");
            if (fgets(line, MAXLINE, f_in) == NULL) {
                err_msg("child closed pipe");
                break;
            }
            line[n] = 0;    /* null terminate */
            if (fputs(line, stdout) == EOF)
                err_sys("fputs error");
        }

        if (ferror(stdin))
            err_sys("fgets error on stdin");
        exit(0);
    } else {                                    /* child */
        close(fd1[1]);
        close(fd2[0]);
        if (fd1[0] != STDIN_FILENO) {
            if (dup2(fd1[0], STDIN_FILENO) != STDIN_FILENO)
                err_sys("dup2 error to stdin");
            close(fd1[0]);
        }

        if (fd2[1] != STDOUT_FILENO) {
            if (dup2(fd2[1], STDOUT_FILENO) != STDOUT_FILENO)
                err_sys("dup2 error to stdout");
            close(fd2[1]);
        }
        // add2.c will be in ./other or ../other depending
        // If you wanted to get fancy you could try something here:
        // https://stackoverflow.com/questions/4025370/can-an-executable-discover-its-own-path-linux
        if (execl("./other/add2", "add2", (char *)0) < 0)
            err_sys("execl error");
    }
    exit(0);
}

static void
sig_pipe(int signo)
{
    printf("SIGPIPE caught\n");
    exit(1);
}
