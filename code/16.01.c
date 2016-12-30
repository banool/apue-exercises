#include <stdio.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    uint16_t num = 123;

    // htons converts to big endian.
    // If the result == the original num, the system must already
    // be big endian. Otherwise the system is little endian.
    if (htons(num) == num) {
        printf("big endian\n");
    } else {
        printf("little endian\n");
    }
}
