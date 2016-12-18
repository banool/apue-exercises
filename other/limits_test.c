#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// See figure 2.11 for the codes to supply to sysconf.
// See figure 2.12 for the codes to supply to pathconf and fpathconf.
// Note that these values (for pathconf and fpathconf) depend on the
// filesystem type (because they're implmeneted in the C library apparently).
// If the return is -1 and errno is set, name wasn't a constant.
// If the return is -1 and errno isn't set, the value is indeterminate.

int main(int argc, char *argv[]) {
    /*
    ** These are the system wide constants.
    */
    printf("sysconf:   %ld\n", sysconf(_SC_PAGE_SIZE));

    /* 
    ** These are the constants defined for a filepath.
    ** I assume that if defined for a path, they will be defined
    ** for all subpaths too, unless overwritten further down along
    ** the hierarchy. The first arg can also be a filename.
    */
    printf("pathconf:  %ld\n", pathconf(".", _PC_PATH_MAX));

    /*
    ** fpathconf can give you the constants defined for a file descriptor.
    ** You'll notice in this example, where nothing has been opened beforehand,
    ** that only 0, 1 and 2 are valid arguments. These represent stdin, stdout
    ** and stderr. Externally, ou can see the open files for an active process
    ** using the lsof command. E.g.
    **
    ** ps aux | grep    # To get the pid of this process.
    ** lsof -p <pid>    # Returns the open files.
    **
    ** There will be three fds for /dev/ttys000, as well as stuff like the
    ** cwd (current working directory) and stuff in the txt (text) section of
    ** memory, specifically this very code as well as some shared libraries.
    */
    printf("fpathconf: %ld\n", fpathconf(0, _PC_PIPE_BUF));

    // To get the length of a type:
    printf("type len:  %lu\n", sizeof(long));

    // Pausing if you wanna do that lsof thing.
    while(1) {
        sleep(1);
    }
    return 0;
}