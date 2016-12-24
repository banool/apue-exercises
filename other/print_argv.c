#include <stdio.h>
#include <stdlib.h>

int main(int charc, char *argv[]) {
	char **ptr;
	for (ptr=argv; *ptr!=NULL; ptr++) {
		printf("%s\n", *ptr);
	}
}