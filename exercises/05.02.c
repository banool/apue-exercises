#include "apue.h"

#define NEW_MAXLINE 4

int
main(void) {
    char buf[MAXLINE];
    while (fgets(buf, NEW_MAXLINE, stdin) != NULL)
        if (fputs(buf, stdout) == EOF)
            perror("output error");
    if (ferror(stdin))
        perror("input error");
    exit(0);
}
