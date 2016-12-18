#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int len_fd_table = getdtablesize();
    printf("Length of fd table (number of files this process can have open at once):\n");
    printf("%d\n", len_fd_table);
    return 0;
}