/* Remember, opendir/fopen/etc. are part of the stdlib (3) but open is a system call (2). */

#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int charc, char *argv[]) {
	DIR *dir;
	int dir_fd;
	
	/* Section 3, stdio. POSIX mandates that the close-on-exec flag must be set */
	dir = opendir("/");
	dir_fd = dirfd(dir);
	printf("close on exec flag: %d\n", fcntl(dir_fd, F_GETFD));
	closedir(dir);
	
	/* Section 2, system calls. */
	dir_fd = open("/", O_RDONLY);
	printf("close on exec flag: %d\n", fcntl(dir_fd, F_GETFD));
	
	return 0;
}
