#include "apue.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
//-#include "signalintr.c"

// 1gb
#define BUFSIZE 1000000000
char buf[BUFSIZE];
// You have to put this here (this is the data segment).
// If you put it in main, it'll be on the stack. If you check
// ulimit -a, you'll see that it is by default not large enough
// for a buffer of 1gb.

void my_handler(int arg);

int main(int argc, char *argv[]) {

	struct rlimit my_limit;
	my_limit.rlim_cur = 1000000000;
	my_limit.rlim_max = 1000000000;
	if (setrlimit(RLIMIT_FSIZE, &my_limit) < 0) {
		perror("setrlimit");
		return -1;
	}

	signal_intr(SIGALRM, my_handler);

	FILE *f = fopen("blah", "wb+");

	alarm(2); // Make sure this is shorter than how long the fwrite takes

	int written = fwrite(buf, 1, BUFSIZE, f);

	if (ferror(f))
		printf("ferror set, there was an error\n");
	else
		printf("ferror not set, disregard the errno below\n");
	printf("Wrote %d bytes, errno = %d msg: %s\n", written, errno, strerror(errno));

}

void my_handler(int arg) {
	printf("Caught %d: %s\n", arg, strsignal(arg));
}
