#include <sys/types.h> // For portability
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
//-#include "stat_info.c"

extern void print_stat_info(struct stat statbuf);

int main(int argc, char *argv[]) {
    int sd;
    struct stat statbuf;

    if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return -1;       
    }
    if (fstat(sd, &statbuf) < 0) {
        perror("fstat error");
        return -1;
    }
    print_stat_info(statbuf);
}
