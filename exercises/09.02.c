#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

/* This program demonstrates that the pid, pgid and "sid" of
** the leader of a new session are all the same */

int main(int argc, char *argv[]) {
	pid_t pid;

	if ((pid = fork()) < 0) {
		perror("fork error");
		return -1;
	} else
	if (pid == 0) {
		// Wait for the parent to die.
		while (getppid() != 1) {
			printf("Waiting for parent to die.\n");
			sleep(1);
		}

		pid = setsid();

		printf("pid, pgid and \"sid\" should be the same:\n");
		printf("pid: %d pgid: %d sid: %d\n", getpid(), getpgrp(), getsid(0));

		if ((open("/dev/tty", O_RDWR)) < 0) {
			printf("Child has no controlling terminal!\n");
		} else {
			printf("Child has a controlling terminal!\n");
		}

	} else {
		if ((open("/dev/tty", O_RDWR)) < 0) {
			printf("Parent has no controlling terminal!\n");
		} else {
			printf("Parent still has a controlling terminal!\n");
		}
		_exit(0);
	}
	return 0;
}
