#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    //_exit(SIGABRT);
    kill(getpid(), SIGABRT);
    return 0; // Shouldn't get here.
}