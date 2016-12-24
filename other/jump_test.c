#include <stdio.h>
#include <setjmp.h>

jmp_buf jbuffer;

int main(int charc, char *argv[]) {
    int i = 0;
    printf("%d\n", setjmp(jbuffer));
    while (i < 10) {
        // If you have i++ here, the loop goes through as normal.
        i++;
        longjmp(jbuffer, i);
        // If you have i++ here, it won't terminate.
        //i++;
    }
    return 0;
}