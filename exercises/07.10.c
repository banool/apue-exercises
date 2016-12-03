#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int f1(int val);

int main(int argc, char *argv[]) {
    printf("val = 2: %d <- should equal 1\n", f1(2));
    printf("val = 0: %d <- should equal 6\n", f1(0));
    return 0;
}

int f1(int val) {
    int num = 0;
    int *ptr = &num;
    
    if (val == 0) {
        int val;
        val = 5;
        ptr = &val;
    }
    int i = 0;
    char *waste;
    while (i < 200) {
        waste = calloc(100000, 100000);
        waste[80000] = 1;
        usleep(50000);
        i++;
    }
    
    return (*ptr + 1);
}
