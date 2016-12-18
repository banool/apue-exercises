#include <stdio.h>
#include "buftype.h"

void mysetbuf(FILE *restrict stream, char *restrict buf);

int main(int charc, char *argv[]) {
    // Print the buf type of the output stream.
    pr_stdio("stdout", stdout);
    // Turn off buffering for stdout.
    mysetbuf(stdout, NULL);
    // Print the buf type of the stream now.
    pr_stdio("stdout", stdout);
    return 0;
}

void mysetbuf(FILE *restrict stream, char *restrict buf) {
    // buf ? _IOFBF : _IONBF
    // If buf, set fully buffered, otherwise turn off buffering.
    // This conforms to the setbuf behaviour:
    // If buf != NULL, set buffer to the provided buffer (must be BUFSIZ).
    // If buf == NULL, turn buffering off.
    // Even though this specifies fully buffered, the system might choose
    // line buffering anyway if appropriate (e.g. terminal as stdin/out).
    if (setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ) < 0)
        perror("setvbuf error");
}    
