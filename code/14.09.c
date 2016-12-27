#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/uio.h>

#define START_BYTES 25
#define END_BYTES 3000000

void timespec_diff(struct timespec *start, struct timespec *stop,
         struct timespec *result);

int main(int argc, char *argv[]) {
    struct timespec start, end, result;

    int fd = open("testbois", O_CREAT|O_TRUNC|O_WRONLY, 0777);
    int fdrand = open("/dev/random", O_RDONLY);

    for (int b = START_BYTES; b <= END_BYTES; b *= 2) {
        char buf1[b];
        char buf2[b];

        // This shouldn't really do anything.
        read(fdrand, buf1, b);
        read(fdrand, buf2, b);

        // Manual copy and write.
        clock_gettime(CLOCK_REALTIME, &start);

        char buf3[b*2];
        memcpy(buf3, buf1, b);
        memcpy(buf3+b, buf2+b, b);

        write(fd, buf3, b*2);

        clock_gettime(CLOCK_REALTIME, &end);
        timespec_diff(&start, &end, &result);
        printf("%7d copy and write: %lld.%.9ld", b, (long long)result.tv_sec, result.tv_nsec);

        ftruncate(fd, 0);

        // Using writev.
        read(fdrand, buf1, b);
        read(fdrand, buf2, b);

        clock_gettime(CLOCK_REALTIME, &start);

        struct iovec bufs[2];
        bufs[1].iov_base = buf1;
        bufs[1].iov_len  = b;
        bufs[2].iov_base = buf2;
        bufs[2].iov_len  = b;

        writev(fd, bufs, 2);

        clock_gettime(CLOCK_REALTIME, &end);
        timespec_diff(&start, &end, &result);
        printf(" writev: %lld.%.9ld\n", (long long)result.tv_sec, result.tv_nsec);

        ftruncate(fd, 0);
    }
    return 0;
}

// Thanks: https://gist.github.com/diabloneo/9619917
void timespec_diff(struct timespec *start, struct timespec *stop,
         struct timespec *result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}
