//-#include "error.c"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "apue.h"

int main(int charc, char * argv[]) {
	
	pid_t pid;
	char buf[20];
	
	if ((pid = fork()) < 0) {
		err_sys("fork");
	} else
	if (pid == 0) {
		printf("this is the child, trying to exit!\n");
		exit(EXIT_SUCCESS);
	} else {
		printf("this is the parent, not waiting for the child\n");
		sprintf(buf, "ps %ld", (long int)pid);
		system(buf);
	}
	return 0;
}
