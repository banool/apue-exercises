#include <stdlib.h>
#include <stdio.h>

void meme(void *buf, size_t size);

int main(int charc, char *argv[]) {
    printf("char:  %ld\n", sizeof(char));
    printf("void*: %ld\n", sizeof(void*));
    return 0;
}