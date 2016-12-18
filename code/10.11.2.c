// From fileio/mycat.c
#include "apue.h"
#include <signal.h>
#include <sys/resource.h>
//-#include "signalintr.c"
//-#include "error.c"

#define	BUFFSIZE	100

void my_handler(int arg);

int
main(void)
{
	int		n;
	int     written;
	char	buf[BUFFSIZE];

	struct rlimit my_limit;
	my_limit.rlim_cur = 2048;
	my_limit.rlim_max = 10000;
	setrlimit(RLIMIT_FSIZE, &my_limit);

	// SIGXFSZ File size limit exceeded (4.2BSD)
	signal_intr(SIGXFSZ, my_handler);

	while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0) {
		if ((written = write(STDOUT_FILENO, buf, n)) != n) {
		    fprintf(stderr, "Wrote %d bytes\n", written);
			err_sys("write error");
		}
	}

	if (n < 0)
		err_sys("read error");

	exit(0);
}

// arg is the signal value, e.g. 14 for SIGALRM.
void my_handler(int arg) {
	printf("Caught %d, %s\n", arg, strsignal(arg));
}

