#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Uncomment one of the two below to see the termination status of each.
    //_exit(SIGABRT);
    //kill(getpid(), SIGABRT);
    return 0; // Shouldn't get here.
}
