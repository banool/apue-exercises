// Remember, this line below will check ../lib for such a file.
#include "apue.h"
//-#include "tellwait.c"
//-#include "error.c"

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

void sig_int(int arg);

FILE *tmp;

// If you had this, it would also not be shared.
// You have to do something special to share vars like this.
//int counter = 1;

static volatile sig_atomic_t sigflag; /* set nonzero by sig handler */ 
static sigset_t newmask, oldmask, zeromask; 
static void sig_usr (int signo) { sigflag = 1; }

int main(int argc, char *argv[]) {
    
    // For some reason the tmpfile isn't working.
    
    if ((tmp = tmpfile()) == NULL) {
        err_sys("tmpfile error");
    }

    //if ((tmp = fopen("tmpfilebois", "w+")) == NULL)
    //    err_sys("fopen error");

    printf("value of close on exec flag: %d (sanity check)\n", fcntl(fileno(tmp), F_GETFD));

    putc('0', tmp);
    putc('\n', tmp);
    fflush(tmp); 
    // Without this fflush, the 0 might still be in the buffer after the fork so
    // it might get written to tmp twice.

    int counter = 1;
    
    pid_t pid;

    TELL_WAIT();
    
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else
    if (pid == 0) {

        // Don't put the signal catching code in the child, let it die.

        for (;;) {
            // Make the child write first yeah?
            // If the parent writes first the child might not be waiting yet?
            WAIT_PARENT();
            printf("child writing %d\n", counter);
            fprintf(tmp, "c%d\n", counter);
            fflush(tmp);
            counter++;
            sleep(1);
            TELL_PARENT(getppid());
        }  
    } else {

        // Only put the signal catching code in the parent, so the handler
        // is only run once and the stuff is only printed out once.

        struct sigaction sig_int_act;

        sig_int_act.sa_handler = sig_int;
        sigemptyset(&sig_int_act.sa_mask);
        sig_int_act.sa_flags = 0;

        if (sigaction(SIGINT, &sig_int_act, NULL) < 0) {
            err_sys("sigaction SIGINT error");
        }

        TELL_CHILD(pid); // To get past the first wait in the child.
        for (;;) {
            // Make the child write first yeah?
            // If the parent writes first the child might not be waiting yet?
            WAIT_CHILD();
            printf("parent writing %d\n", counter);
            fprintf(tmp, "p%d\n", counter);
            fflush(tmp);
            counter++;
            sleep(1);
            TELL_CHILD(pid);
        }
    }
    return 0;
}

void sig_int(int arg) {
    fseek(tmp, 0, SEEK_SET);
    char c;
    while ((c = getc(tmp)) != EOF) {
        putchar(c);
    }

    struct sigaction dfl_act;

    dfl_act.sa_handler = SIG_DFL;
    sigemptyset(&dfl_act.sa_mask);
    dfl_act.sa_flags = 0;

    // Reset SIGINT to its dfl_act action and resend it.
    if (sigaction(SIGINT, &dfl_act, NULL) < 0) {
        err_sys("sigaction SIGINT error");
    }

    kill(getpid(), SIGINT);

}

