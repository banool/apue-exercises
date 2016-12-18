#include "apue.h"
//-#include "signalintr.c"
#include <unistd.h>

void my_handler(int arg);

int main(int argc, char *argv[]) {
	signal_intr(SIGALRM, my_handler);
	alarm(1);
	sleep(3);
	return 0;
}

void my_handler(int arg) {
	int i = 0;
	for(;;) {
		printf("%d in signal handler %d\n", i++, arg);
		sleep(1);
		alarm(1); // This won't do anything.
		if (i > 5) {
			kill(getpid(), SIGINT); // This will.
		}
	}
}
