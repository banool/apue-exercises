#include <stdio.h>
#include <sys/select.h>

int sleep_us(int ms);

int main(int argc, char *argv[]) {
    printf("Waiting 2 seconds\n");
    sleep_us(2000000);
    return 0;
}

int sleep_us(int ms) {
    struct timeval tv;
    // This is better than just putting all the ms in tv_usec.
    tv.tv_sec  = ms / 1000000;
    tv.tv_usec = ms % 1000000;
    // Will return 0 on success since all the sets are NULL.
    return select(0, NULL, NULL, NULL, &tv);
}
