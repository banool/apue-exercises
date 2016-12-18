#include <time.h>
#include <stdio.h>
#include <stdlib.h> // For setenv

#define BUF_SIZE 64

int main(int charc, char *argv[]) {
    char buf[BUF_SIZE];

    time_t sec_since_epoch = time(NULL);
    struct tm tmstruct = *localtime(&sec_since_epoch);
    if (strftime(buf, BUF_SIZE, "%a %e %b %Y %I:%M:%S %Z", &tmstruct) == 0) {
        perror("buffer too small");
        return -1;
    }
    printf("%s\n", buf);
    
    // Changing TZ to UTC. It defaults to UTC.
    setenv("TZ", "lsahglkdsad", 1);
    tmstruct = *localtime(&sec_since_epoch);
    if (strftime(buf, BUF_SIZE, "%a %e %b %Y %I:%M:%S %Z", &tmstruct) == 0) {
        perror("buffer too small");
        return -1;
    }
    printf("%s\n", buf);
    return 0;
}
