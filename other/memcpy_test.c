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
*/

int main() {
    char buf[32];
    long code = 4;
    unsigned long bytes_long = sizeof(long) / 8;
    memcpy(buf, &code, bytes_long);
    memcpy(buf+bytes_long, "hello!\0", 7);
    printf("size of a long: %lu byte(s)\n", bytes_long);
    for (int i = 0; i < 32; i++){
        printf("%d buf is: %02o\n",i,buf[i]);
    }
    printf("message after code: %s\n", buf+bytes_long);
    return 0;
}
