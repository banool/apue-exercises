#include <stdio.h>

int main(int argc, char *argv[]) {
    FILE *fp = popen("mynonsensecommand", "r");
    if (fp == NULL) {
        printf("popen returned a null pointer\n");
    } else {
        printf("popen returned a non-null pointer\n");
    }
    return 0;
}
