#include "apue.h"
//-#include "prexit.c"
//-#include "error.c"
#include <sys/wait.h>
#include <string.h> // for strsignal

#define MYFLAGS WEXITED | WSTOPPED | WCONTINUED

void print_info(siginfo_t *info);

int
main(void)
{
	// int waitid(idtype_t idtype, id_t id, siginfo_t *infop, int options);
	pid_t	pid;
	siginfo_t  info;

	if ((pid = fork()) < 0)
		err_sys("fork error");
	else if (pid == 0)				/* child */
		exit(7);

	if (waitid(P_PID, pid, &info, MYFLAGS) != 0)		/* wait for child */
		err_sys("wait error");
	print_info(&info);				/* and print its status */

	if ((pid = fork()) < 0)
		err_sys("fork error");
	else if (pid == 0)				/* child */
		abort();					/* generates SIGABRT */

	if (waitid(P_PID, pid, &info, MYFLAGS) != 0)		/* wait for child */
		err_sys("wait error");
	print_info(&info);				/* and print its status */

	if ((pid = fork()) < 0)
		err_sys("fork error");
	else if (pid == 0)				/* child */
		pid /= 0;				/* divide by 0 generates SIGFPE */

	if (waitid(P_PID, pid, &info, MYFLAGS) != 0)		/* wait for child */
		err_sys("wait error");
	print_info(&info);					/* and print its status */

	exit(0);
}

void print_info(siginfo_t *info) {
    if (info->si_code == CLD_EXITED) {
        printf("normal termination, exit status %d\n", info->si_status);
     } else {
        printf("abnormal termination, signal number = %d %s\n", 
	    info->si_status, strsignal(info->si_status));
    }
}
	

