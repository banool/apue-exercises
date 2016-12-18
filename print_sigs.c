#include <signal.h>
#include <stdio.h>

int main(int charc, char * argv[]) {
    // 0 isn't actually a signal ofc.
    // TODO find the constant for the number of sigs.
    // Without posix defined it's __DARWIN_NSIGS.
    // No access to google rn.
    // dw its NSIG lol, though I couldn't find it in
    // /usr/include/signal.h or /usr/include/sys/signal.h
    char buf[16];
    for (int i = 0; i < NSIG; i++) {
        snprintf(buf, 16, "Signal %2d", i);
        psignal(i, buf);
    }
    return 0;
}