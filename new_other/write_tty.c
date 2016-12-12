#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

/* 9.6 */

/* This program demonstrates that the pid, pgid and "sid" of
** the leader of a new session are all the same */

int main(int argc, char *argv[]) {
	int fd;

	/* We use O_NOCTTY. The process should already have a session controlling
	** terminal, but this flag just makes sure we don't adport the
	** target as controlling terminal even if we don't have one. */
	/* UPDATE: We don't need the O_NOCTTY flag. If the process doesn't have
	** a controlling terminal, the special file /dev/tty won't exist. */
	if ((fd = open("/dev/tty", O_WRONLY)) < 0) {
		perror("open error");
		return -1;
	}

	write(fd, "writing directly to /dev/tty\n", 30);
	return 0;
}