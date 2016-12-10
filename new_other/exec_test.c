#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int charc, char *argv[]) {
	if (setenv("PATH", getcwd(NULL, 0), 1) == -1) {
		perror("setenv error");
		return -1;
	}

	/*printf("path: %s\n", getenv("PATH"));*/

	if (execlp("print_argv", "my arg", (char*)NULL) == -1) {
		perror("execl error");
		return -1;
	}
}