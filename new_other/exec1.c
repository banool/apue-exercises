#include "apue.h"
#include <sys/wait.h>

char	*env_init[] = { "USER=unknown", "PATH=/tmp", NULL };

int
main(void)
{
	pid_t	pid;

	if (setenv("PATH", getcwd(NULL, 0), 1) == -1) {
		perror("setenv error");
		return -1;
	}

	if ((pid = fork()) < 0) {
		perror("fork error");
	} else if (pid == 0) {	/* specify pathname, specify environment */
		if (execle("/home/daniel/Desktop/apue/mystuff/print_argv", "print_argv", "myarg1",
				"MY ARG2", (char *)0, env_init) < 0)
			perror("execle error");
	}

	if (waitpid(pid, NULL, 0) < 0)
		perror("wait error");

	if ((pid = fork()) < 0) {
		perror("fork error");
	} else if (pid == 0) {	/* specify filename, inherit environment */
		if (execlp("print_argv", "print_argv", "only 1 arg", (char *)0) < 0)
			perror("execlp error");
	}

	exit(0);
}
