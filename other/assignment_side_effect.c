#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    char n;
    // Proves (I think) that the side effect of assignment
    // 
    printf("%d\n", (n='a'));
    return 0;
}