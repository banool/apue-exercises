#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_bin(int n);

int main(int argc, char *argv[]) {
    int a = 0b1010;
    int b = 0b1011;
    // a = a | b;
    a |= b;
    printf("a |= b: ");
    print_bin(a);
    printf("\n");

    a = 0b1010;
    // a = a & b;
    a &= b;
    printf("a &= b: ");
    print_bin(a);
    printf("\n");

    return 0;
}

void print_bin(int n) {
    char stack[4];
    int i = 3;
    while (n) {
        if (n & 1)
            stack[i] = 1;
        else
            stack[i] = 0;

        n >>= 1;
        i -= 1;
    }

    i++;

    while(i < 4) {
        printf("%d", stack[i]);
        i++;
    }
}