#include <unistd.h>
#include <stdio.h>

/* 9 */

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
		printf("pid: %d pgid: %d sid: %d\n", getpid(), getpgid(0), getsid(0));

	} else {
		_exit(0);
	}
	return 0;
}