#include <stdio.h>
#include <string.h>

/*
This little program demonstrates how you might build a simple struct
manually, such as for the message in msgsnd (see exercise 15.12). We 
copy in a code of size long, then after that many bytes (sizeof(long)/8) 
we copy in a message with a sentinel on the end. A long on this system 
only takes 1 byte, so when we print out each byte of memory (in octal 
format) we see the code 4 in the first byte, then the octal ascii codes 
of the message and null byte in the next 7 bytes (followed by random
memory). We finally print the message following the long code.

UPDATE: The above is wrong, and was leading to lots of problems in 15.12.
sizeof returns the size in BYTES, not bits, hence a long does indeed take
up 8 bytes, meaning the max size of a long is (2^8)^8. The below example
only works because the code 4 is small enough to fit in the first byte.
Note also that the 4 is in the first byte, not the last (7th) byte. This
is because this system is little endian: https://en.wikipedia.org/wiki/Endianness

Below has been updated to have the incorrect and then correct code.
*/

#define BUFSIZE 32

void wrong();
void right();

int main() {
    wrong();
    right();
    return 0;
}

void wrong() {
    char buf[BUFSIZE];
    long code = 4;
    unsigned long bytes_long = sizeof(long) / 8;
    memcpy(buf, &code, bytes_long);
    memcpy(buf+bytes_long, "hello!\0", 7);
    printf("size of a long: %lu byte(s)\n", bytes_long);
    for (int i = 0; i < BUFSIZE; i++){
        printf("%d buf is: %02o\n",i,buf[i]);
    }
    printf("message after code: %s\n", buf+bytes_long);
}

void right() {
    char buf[BUFSIZE];
    long code = 4;
    unsigned long bytes_long = sizeof(long); // This is the fix.
    memcpy(buf, &code, bytes_long);
    memcpy(buf+bytes_long, "hello!\0", 7);
    printf("size of a long: %lu byte(s)\n", bytes_long);
    for (int i = 0; i < BUFSIZE; i++){
        printf("%d buf is: %02o\n",i,buf[i]);
    }
    printf("message after code: %s\n", buf+bytes_long);
}
