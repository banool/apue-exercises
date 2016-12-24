#include	<sys/wait.h>
#include	<errno.h>
#include	<unistd.h>
#include	<stdio.h>

int mysystem(const char *cmdstring);

int main(int argc, char *argv[]) {
	int ret;

	if ((ret = mysystem("date")) < 0) {
		perror("mysystem() error");
		// Don't return tho.
	}

	if ((ret = mysystem("nosuchcommand")) < 0) {
		perror("mysystem() error");
	}

	if ((ret = mysystem("echo well execl does indeed break\
		up the string into separate arguments by space")) < 0) {
		perror("mysystem() error");
	}


}

int
mysystem(const char *cmdstring)	/* version without signal handling */
{
	pid_t	pid;
	int		status;

	if (cmdstring == NULL)
		return(1);		/* always a command processor with UNIX */

	if ((pid = fork()) < 0) {
		status = -1;	/* probably out of processes */
	} else if (pid == 0) {				/* child */
		execl("/bin/sh", "sh", "-c", cmdstring, (char *)0);
		_exit(127);		/* execl error */
	} else {							/* parent */
		while (waitpid(pid, &status, 0) < 0) {
			if (errno != EINTR) {
				status = -1; /* error other than EINTR from waitpid() */
				break;
			}
		}
	}

	return(status);
}
