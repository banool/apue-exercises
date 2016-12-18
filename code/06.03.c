#include <sys/utsname.h>
#include <stdio.h>

int main(int charc, char *argv[]) {
    struct utsname u;
    if (uname(&u) < 0) {
        perror("uname error");
        return -1;
    }
    
    printf("sysname:  %s\n", u.sysname);
    printf("nodename: %s\n", u.nodename);
    printf("release:  %s\n", u.release);
    printf("version:  %s\n", u.version);
    printf("machine:  %s\n", u.machine);
    return 0;
}
