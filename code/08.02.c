// vforking from a chile
#include <unistd.h>
#include <stdio.h>

void myfunc();

int main(int charc, char *argv[]) {
    if ((setvbuf(stdout, NULL, _IONBF, 0)) != 0) {
        perror("setvbuf error");
        return -1;
    }
    printf("main here!\n");
    myfunc();
    printf("back to main!\n");
    return 2;
}

void myfunc() {
    pid_t pid;
    
    if ((pid = vfork()) < 0) {
        perror("vfork error");
        return;
    } else
    if (pid == 0) { // Child
        printf("sup dog it's the child after vfork\n");
        // If you don't have something like
        // _exit(0);
        // here, you're going to get a segfault.
    } else {        // Parent
        printf("parent after vfork!\n");
    }
    // At this point both the parent and child will try to implicitly return.
    // You could also have an explicit return, it would still fuck up.
}
