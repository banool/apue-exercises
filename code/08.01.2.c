#include "apue.h"

int globvar = 6;        /* external variable in initialized data */

int
main(void)
{
    int     var;        /* automatic variable on the stack */
    pid_t   pid;

    var = 88;
    printf("before vfork\n");   /* we don't flush stdio */
    if ((pid = vfork()) < 0) {
        perror("vfork error");
        return -1;
    } else if (pid == 0) {      /* child */
        globvar++;               /* modify parent's variables */
        var++;
        fclose(stdout);
        exit(0);               /* child terminates */ // NOTE
    }

    /* parent continues here */
    printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar,
      var);
    exit(0);
}
