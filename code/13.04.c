#include "apue.h"
#include <unistd.h>
#include <fcntl.h>

//-#include "daemonize.c"
//-#include "error.c"

#define OUTFILE "/home/daniel/myout.txt"
#define BUFSIZE 32

int main(int argc, char *argv[]) {
	int fd;
	char lname[BUFSIZE];

	daemonize(argv[0]);

	if ((fd = open(OUTFILE, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP)) < 0) {
		perror("open error");
		return -1;
	}

	if (getlogin_r(lname, BUFSIZE) != 0) {
		perror("getlogin error");
		return -1;		
	}

	if (write(fd, lname, strlen(lname)) < 0) {
		perror("write error");
		return -1;			
	}

	return 0;
}
