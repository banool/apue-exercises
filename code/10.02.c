#include <signal.h>
#include <stdio.h>
#include <string.h>

int sig2str(int signo, char *str);

int main(int argc, char *argv[]) {
    char buf[32];
    sig2str(5, buf);
    printf("sig 5: %s\n", buf);
    return 0;
}


int sig2str(int signo, char *str) {
    if (signo < 0 || signo > NSIG) {
        return -1;
    }
    
    // It's up to the caller to make sure there is
    // enough room in str for the return of strsignal
    // (plus a null byte \0).
    strcpy(str, strsignal(signo));
    
    return 0;
}
