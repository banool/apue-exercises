#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

// Prototypes
int putenv_r(char *string);

// Global vars / mutex stuff
extern char **environ;
pthread_mutex_t env_mutex = PTHREAD_MUTEX_INITIALIZER;

int
main(int argc, char *argv[]) {
	
	int ret = putenv_r("key1=myvalue friend");
	printf("%d: mykey = %s\n", ret, getenv("key1"));

	ret = putenv_r("key2=myvalue dog");
	ret = putenv_r("key3=myvalue dog");
	ret = putenv_r("key4=myvalue dog");
	ret = putenv_r("key5=myvalue dog");
	ret = putenv_r("key6=myvalue dog");
	ret = putenv_r("key7=myvalue dog");
	ret = putenv_r("key8=myvalue dogs");
	printf("%d: mykey = %s\n", ret, getenv("key8"));

	return 0;
}


int
putenv_r(char *string)
{
    int len;
	int key_len = 0;
	int i;

	sigset_t block;
	sigset_t old;

	sigfillset(&block);
	pthread_sigmask(SIG_BLOCK, &block, &old);

	// This function is thread-safe
	len = strlen(string);

	// Like glibc 2.1.2 and onwards we don't make a copy of the string.
	// The below behaviour, making a copy, is what older glibcs did.
	/*
	char pair[BUFSIZE];
	strncpy(pair, string, len);
	pair[len] = 0;
	*/

	for (int i=0; i < len; i++) {
		if (string[i] == '=') {
			key_len = i;
			break;
		}
	}
	
	// Need a string like key=value
	if (key_len == 0) {
		errno = EINVAL; // putenv doesn't normally return this err code
		return -1;
	}

	// We're moving into environ territory so start locking stuff up.
	pthread_mutex_lock(&env_mutex);

	for (i = 0; environ[i] != NULL; i++) {
		if (strncmp(string, environ[i], key_len) == 0) {
			// Pointer assignment, so if string changes so does the env.
			environ[i] = string;
			pthread_mutex_unlock(&env_mutex);
			return(0);
		}
	}

	int n = sizeof(environ)/sizeof(environ[0]);
	printf("%d   %s\n", n, environ[i-1]);

	// If we get here, the env var didn't already exist, so we add it.
	// Note that malloc isn't async-signal safe. This is why we block signals.
	environ[i] = malloc(sizeof(char *));
	environ[i] = string;
	environ[i+1] = NULL;
	// This ^ is possibly incorrect, do I need to grow environ somehow?

	pthread_mutex_unlock(&env_mutex);
	pthread_sigmask(SIG_SETMASK, &old, NULL);

	return(0);
}

