#include <stdio.h>

int main(int charc, char *argv[]) {
    int n = printf("Printing a string\n");
    printf("Return value of previous print: %d\n", n);
    printf("The return value of printf is the number of chars printed, not including the null byte.\n");
    return 0;
}
