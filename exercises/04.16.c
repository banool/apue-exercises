#include "apue.h"
#include <fcntl.h>

int
main(int argc, char *argv[])
{
    const char* dirname = "mydir";

    long pathmax = pathconf(".", _PC_PATH_MAX);
    printf("pathmax: %ld\n", pathmax);
    char path[pathmax];
    long curr_len;
    // We keep looping until we exceed pathmax.
    while ((curr_len = strlen(getcwd(path, pathmax))) < pathmax) {
        // DIR_MODE is defined in apue.h
        // Note, mkdir returns 0 on success, not the fd of the dir.
        if (mkdir(dirname, DIR_MODE) < 0) {
            perror("mkdir error, did you make sure mydir didn't exist before running");
            return -1;
        }
        chdir(dirname);
        printf("Current absolute path length: %ld\n", curr_len);
    }
    
    return 0;
}
