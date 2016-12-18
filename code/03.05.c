#include <stdio.h>
#include <unistd.h>

int main() {
    fprintf(stdout, "to stdout\n");
    fflush(stdout);
    fprintf(stderr, "to stderr\n");
    return 0;
}
