# Exercises

For code you don't want compiled, do triple dash c NO:
`dash` `dash` `dash` `c` `[space]` `N` `O`.
See exercise 4.11 for an example.

You should have all of the source code from [here](http://www.apuebook.com/code3e.html) handy.

The word **TODO** is used to denote things I haven't done yet or wasn't sure about.

#  3 File I/O
## 3.1
No, the data still passes through the kernel block buffers. Also known as the
kernel's buffer cache. So really, this isn't unbuffered I/O, but refers to the
lack of buffering that occurs in the user space functions. Some systems provide
a direct I/O to bypass these kernel buffers. 

## 3.2
```c
#include <stdio.h>
#include <unistd.h>

int main() {
    dup2(1, 5);
    // Check out the open fds with lsof.
    while(1) {
        sleep(1);
    }
    return 0;
}

int dup2(int fd1, int fd2) {

    if (fd1 == fd2)
        return fd2;

    close(fd2);

    // TODO how to duplicate without dup or fnctl?
    // Just opening fd2 will open a new file descriptor no? It won't point to
    // the same file table entry as fd1 yeah?

    // How about this: Keep duplicating until we get the fd we wanted, store
    // the file descriptors it returned in an array (so we don't close any fds
    // that were already open), and then close all the fds we opened on the way
    // to fd2.

    // We make an array of fds with the number of slots corresponding
    // to fd2. This leaves us enough space for the fds we create with
    // dup all the way up to fd2, though we likely won't need all of
    // the slots, since some fds will be in use (stdin/out/err).
    int fds[fd2];
    int num_made = 0;
    int descriptor = -1;
    // Duplicate descriptors up until we get fd2.
    while (descriptor != fd2) {
        descriptor = dup(fd1);
        fds[num_made] = descriptor;
        num_made++;
    }

    // Close all the fds we opened up until fd2 (hence the -1).
    for (int i = 0; i < num_made - 1; i++) {
        close(fds[i]);
    }

    return descriptor;
}
```

## 3.3

PROCESS TABLE      FILE TABLE           VNODE TABLE

fd1 --|
       ----- file table entry 1 --|
fd2 --|                           |
                                  --- vnode entry
fd3 -------- file table entry 2 --|

## 3.4

If fd is 0, 1 or 2, fd will be duplicated without closing fd.
However, if fd is > 2, fd won't be closed automatically. To get
consistent behaviour, we close fd if > 2 (so that it there are 
3 file descriptors).'

TODO don't quite get this, what's the point? The code obviously
redirects stdin, stdout and stderr to fd. Or does it?

## 3.5
TODO brush up on this.

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    fprintf(stdout, "to stdout\n");
    fflush(stdout);
    fprintf(stderr, "to stderr\n");
    return 0;
}
```

Try:
`./03.05 > out`
You'll see that *to stdout* will be written to out, while *to stderr*
will be written to the terminal. The `>` changes stdout from the
terminal to *out*.

Now try:
`./03.05 > out 2>&1`
Both stdout and stderr will be in *out*. This is because fd2 (stderr)
is redirected to fd1 (stdout). The order might not be preserved,
because stdout is sometimes buffered while stderr is never buffered.
This is why we include the line `fflush(stdout)`.

Now this:
`./03.05 2>&1 > out`
In this, stderr is redirected to stdout. stdout is then redirected to *out*. This does not mean that stderr is also redirected to out. The line *to stderr* will be printed to stdout (the terminal) while *to stdout* will be written to *out*.

## 3.6
With the O_APPEND flag set, you can still read from anywhere in the file. However, even if you seek to a specific spot in the file, a call to write will automatically move the current file offset to the end of the file before doing the write. This cannot be avoided.

```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main() {
    // Open the file.
    int fd = open("test", O_RDWR|O_CREAT|O_TRUNC|O_APPEND, S_IRWXU);
    // Write to the file.
    printf("Writing first line\n");
    if(write(fd, "first line\n\0", 12) < 0)
        perror("write");
    // lseek to the start, read and print it.
    // Note that this is allowed, even with O_APPEND.
    printf("lseeking to the start and reading/printing\n--\n");
    lseek(fd, 0, SEEK_SET);
    char buf[12];
    read(fd, buf, 12);
    printf("%s", buf);
    // Print the current offset to prove we're at the end.
    printf("Note how current offset is at the end: %d\n", (int)lseek(fd, 0, SEEK_CUR));
    // Now lseek to the start and try to write.
    printf("lseeking to the start.\n");
    lseek(fd, 0, SEEK_SET);
    printf("Note how current offset is at the start: %d\n", (int)lseek(fd, 0, SEEK_CUR));
    printf("Trying to write to the current offset (the start).\n");
    write(fd, "second line\n\0", 13);
    // If we print the file, we'll see that second line printed
    // after first line, not before like we tried. This proves
    // that with O_APPEND set, we can only write to the end.
    printf("Printing the file, you'll see that 2nd line was appended regardless.\n--\n");
    lseek(fd, 0, SEEK_SET);
    char buf2[25];
    if(read(fd, buf2, 25) < 0)
        perror("read");
    if(write(STDOUT_FILENO, buf2, 25) < 0)
        perror("write");
    
    return 0;
}
```

# 4 File directories
## 4.1
```c
#include "apue.h"

int
main(int argc, char *argv[])
{
	int			i;
	struct stat	buf;
	char		*ptr;

	for (i = 1; i < argc; i++) {
		printf("%s: ", argv[i]);
		if (stat(argv[i], &buf) < 0) {
			perror("stat error");
			continue;
		}
		if (S_ISREG(buf.st_mode))
			ptr = "regular";
		else if (S_ISDIR(buf.st_mode))
			ptr = "directory";
		else if (S_ISCHR(buf.st_mode))
			ptr = "character special";
		else if (S_ISBLK(buf.st_mode))
			ptr = "block special";
		else if (S_ISFIFO(buf.st_mode))
			ptr = "fifo";
		else if (S_ISLNK(buf.st_mode))
			ptr = "symbolic link";
		else if (S_ISSOCK(buf.st_mode))
			ptr = "socket";
		else
			ptr = "** unknown mode **";
		printf("%s\n", ptr);
	}
	exit(0);
}
```

The difference is that `stat` follows symlinks, whereas `lstat` looks at the symlink itself. The "symbolic link" `S_ISLNK` trigger will never pop with this on. Even if you have a broken symlink, stat will just error out: **stat error: No such file or directory**. I changed `err_ret` to `perror`, `err_ret` doesn't exist?

## 4.2
With a umask of 777, the file should be made with no permissions enabled. This would be easier to test on linux, doesn't look like osx honours the umask properly, at least when set with `umask 777` from the shell.

## 4.3
Again, probs need a less hand-holdy unix to test this.

## 4.4
Nothing will happen to either, they already exist. The permissions won't be changed either. O_CREAT means only create if it doesn't exist.

## 4.5
No. A symbolic link is exactly as long as the filename that it points to (no need for a sentinel, it's length is bound by the st_size field. A directory is never st_size=0 either, since the directory will always hold at least `.` and `..`.

## 4.6
TODO test this on linux, as macOS doesn't have SEEK_HOLE/SEEK_DATA.

```c
#include "apue.h"
#include <fcntl.h>

#define BUF_SIZE 4096

int
main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s <source> <target>\n", argv[0]);
        return -1;
    }
    // Open the source file for reading.
    int source = open(argv[1], O_RDONLY);
    // Open the target file for writing.
    int target = open(argv[2], O_WRONLY|O_CREAT|O_TRUNC);
    
    // Find the start of the hole.
    off_t curr = 0;
    int in_data = 0;
    off_t marker;
    
    marker = lseek(source, curr, SEEK_HOLE);
    // If marker > curr, that means we started in data.
    if (marker > curr) {
        // Reset the current file offset back to the start.
        lseek(source, curr, SEEK_DATA);
    }
    // Else, if marker == curr, it means there is a hole at the
    // start of the file. We then find when data starts.
    // We also lseek in the target file, making the hole in the target.
    else {
        marker = lseek(source, curr, SEEK_DATA);
        lseek(target, marker, SEEK_SET);
        curr = marker;
        marker = lseek(source, curr, SEEK_HOLE);
    }
    
    // Because either way we start at a data section:
    in_data = 1;
    
    char buf[BUF_SIZE];
    int to_copy;
    while (curr < marker) {
        if (in_data) {
            // Copy data.
            // If we have more than BUF_SIZE left, copy BUF_SIZE.
            if ((marker - curr) > BUF_SIZE) {
                to_copy = BUF_SIZE;
            // Otherwise copy the remaining section of data (< BUF_SIZE).
            // We will now be in a hole, so set in_data = 0;
            } else {
                to_copy = marker - curr;
                in_data = 0;
            }
            read(source, &buf, to_copy);
            write(target, &buf, to_copy);
            curr += to_copy;
        } else {
            // If we are in this block, it means we've hit the end of a
            // data section and are in a hole. As such, we seek for the
            // start of the next data section.
            curr = lseek(source, curr, SEEK_DATA);
            // And then find the end of that data section.
            marker = lseek(source, curr, SEEK_HOLE);
            // This line seeks through the target file, making the hole.
            lseek(target, curr, SEEK_SET);
        }
    }
    
    return 0;
}
```

## 4.7
For different actions and some special file names, the shell and kernel respectively have default umasks.

## 4.8
After the call to unlink, the pathname pointing to the file no longer exists. This means we can't query it with du (directly anyway, we could call du on the parent directory and note that the size has changed). As such, in order to see that (for 15 seconds) the file contents still exist on disk because the process is open, we call df.

## 4.9
The changed time on a file changes when the inode undergoes changes. If the `unlink` function is called on a file with 2 or more links, the link counter will be decremented by one and the changed time will be updated to when `unlink` was called. These access/modification/change times are attached to the inode of course, not the filename which points to it.

## 4.10
TODO the myftw function in question is pretty in depth, might be worth checking out again. The depth of the scan is limited by the max number of file descriptors that a process can hold open. This is because the ancestor directories' fds are left open by the process after each recursive step. To solve this you could close file descriptors if you need to and work relative to a directory further down the directory structure, changing the relative start up and down as we go. Could be hard to keep track of what you've seen so far though.

## 4.11
We won't compile this because it relies on a billion different things to compile. Copy this function into the `ftw8.c` file in the `filedir` folder, it'll compile it with all the necessary functions.

```c NO
// TODO, this would be easier to just work on directly in the original file.
```

## 4.12
`chroot` changes the location of `/`. If I chroot a process to `/home/daniel`, then I can no longer ascend back up the hierarchy. This is obviously handy for jailing people to subdirectories, for example by chrooting a user's login shell process.

You can also use chroot to clone a filesystem, as you've done before to fix a broken file system. Only the superuser can use chroot.

## 4.13
Call `stat` and get the previous 3 times (access/modify/change). For the one we don't want to change, just copy across the value we got from stat (remember, **change** will be changed by the utimes call no matter what, the function only has options for access and modify).

## 4.14
This is a bit archaic, imagine that the mailbox is just a file (or directory, which is still a file). The access time is when you last read the email and the modify time is when mail was last received.

## 4.15
The change time isn't stored because, even if it was stored, you wouldn't be able to set it to the original time. You cannot cheat the change time, it is always based on when the inode data was actually changed.

Depending on the utility (tar or cpio), you can tell it to keep the original access and/or modify times. For example, tar by default maintains the original modify time but you can use the `-m` switch to set it to extraction time. The access time is always set to extraction time.

## 4.16
```c
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
```

This program will eventually throw a segmentation fault (when we surpass pathmax). This only happens on macOS (because of getcwd). In the other 3 major OSes, it is possible to get a pathname longer than pathmax with a big buffer (created with lots of reallocs). However, most functions won't work with a pathname longer than pathmax anyway. `tar` can still archive this long directory hierarchy on all platforms, but in linux it can't be unarchived. TODO good question, interesting.

## 4.17
You can't just unlink something in /dev as a regular user, write permissions are turned off in there. TODO looking at /dev in macOS, it looks like lots of files have write permissions enabled, including /dev/fd/0 (and 1 and 2, but not the stdin/stdout/stderr symlinks?). You should check out how this behaves on linux.

# 5

## 5.1
```c
#include <stdio.h>
#include "buftype.h"

void mysetbuf(FILE *restrict stream, char *restrict buf);

int main(int charc, char *argv[]) {
    // Print the buf type of the output stream.
    pr_stdio("stdout", stdout);
    // Turn off buffering for stdout.
    mysetbuf(stdout, NULL);
    // Print the buf type of the stream now.
    pr_stdio("stdout", stdout);
    return 0;
}

void mysetbuf(FILE *restrict stream, char *restrict buf) {
    // buf ? _IOFBF : _IONBF
    // If buf, set fully buffered, otherwise turn off buffering.
    // This conforms to the setbuf behaviour:
    // If buf != NULL, set buffer to the provided buffer (must be BUFSIZ).
    // If buf == NULL, turn buffering off.
    // Even though this specifies fully buffered, the system might choose
    // line buffering anyway if appropriate (e.g. terminal as stdin/out).
    if (setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ) < 0)
        perror("setvbuf error");
}    
```

## 5.2
```c
#include "apue.h"

#define NEW_MAXLINE 4

int
main(void) {
    char buf[MAXLINE];
    while (fgets(buf, NEW_MAXLINE, stdin) != NULL)
        if (fputs(buf, stdout) == EOF)
            perror("output error");
    if (ferror(stdin))
        perror("input error");
    exit(0);
}
```

It still copies the stdin to stdout fine. It also still breaks up input/output by newline. The difference is that, if a line is longer than 4 chars, it will be broken up into multiple buffers of length 4 until we hit the newline (which is included in the buffer remember). Each time we fill a buffer, it is a new call to `fgets`. `fputs` doesn't care about newlines in buffer, it just stops upon hitting a null byte `'\0'`. This is in contrast to gets and puts, which would fail if the buffer wasn't long enough for the whole line.

## 5.3
```c
#include <stdio.h>

int main(int charc, char *argv[]) {
    int n = printf("Printing a string\n");
    printf("Return value of previous print: %d\n", n);
    printf("The return value of printf is the number of chars printed, not including the null byte.\n");
    return 0;
}
```

A return value of 0 means 0 chars printed.

## 5.4
`getc` and `getchar` return an int, not a char. EOF is normally defined to be `-1`. This isn't a problem on systems where chars are a signed type, but when it is unsigned then the termination clause (when c == EOF) might never trigger because the return of `getc`/`getchar` might not equal the EOF value anymore. The 4 major systems discussed here use signed chars tho so no worries.

## 5.5
I imagine you would have to open the FILE* struct and get the file descriptor from inside it.

Nope ^. You can use the `fileno` function to get the file descriptor of a FILE* struct. Make sure to call `fflush` before `fsync`, so that the user space buffers (stdio libs) are flushed to the kernel buffers first. `fsync` then flushes the kernel buffers to disk.

## 5.6
When `fgets` is called (line by line function), anything in the stdout buffer is flushed.

## 5.7
```c NO
#include <stdio.h>

FILE *fmemopen(void *buf, size_t size, const char *mode);
```

This is a pretty hefty question, but probably worth doing. TODO.
In the code for this question in the answers (as well as a lot previously) we see the `restrict` keyword used quite often. [This](https://en.wikipedia.org/wiki/Restrict) wikipedia article is actually very illuminating/clear about how it works.

Clarification about set-uid-bit: If this bit is set for an executable file, then whenever a user other than the owner executes the file, that user acquires all the file read/write/execute privileges of the owner in accessing any of the owner's other files!

See page 181. Have a good think about this and you'll see why set-uid is secure. If the set-uid-bit is set on a program, when a user runs that program, that process instance will have the permissions of the program owner. This means that through this process, the user will have access to whatever the program owner has access too. 

The login and passwd programs are set-uid programs, and their owner is root. This means that a user running these programs will have root permissions, but only through that program. This means that, even though the process has root permissions, the user can only do whatever functionality the program has defined. A user can't make a root set-uid program themselves without root permissions. However this means that if there are any vulnerabilities in a root set-uid program, then the whole system is at risk.

# 6
## 6.1
Depends on the implementation. On Mac OS X you can't get to the encrypted password (the regular password file is shadowed automatically). On Linux there are a bunch of functions which can access the shadow password but you need superuser privileges to use them. The shadow password files aren't accessible by the world.

## 6.2
Unfortunately I'm on Mac OS X but this code should hopefully work on Linux.

```c NO
#include <shadow.h>

int main(int charc, char *argv[]) {
    char buf[32];
    get_encrypted_pword(&buf, 32, "daniel");
    printf("encrypted pword: %s\n", buf);
    return 0;
}

int get_encrypted_pword(char *buf, int size, const char *name) {
    struct spdw shadow;
    shadow = getspnam(name);
    int len_pwd = strlen(shadow->sp_pwdp);
    if (len_pwd > (size-1))
        return -1;
    // +1 so a null byte is appended.
    strncpy(buf, shadow->sp_pwdp, len_pwd+1);
    return len_pwd;
}
```

Here is a similar example to get the description of a user under Mac OS X.

```c
#include <sys/types.h>
#include <pwd.h>
#include <uuid/uuid.h>
#include <string.h>
#include <stdio.h>

int get_encrypted_pword(char *buf, int size, const char *name);

int main(int charc, char *argv[]) {
    char buf[32];
    get_encrypted_pword(buf, 32, "daniel");
    printf("User description: %s\n", buf);
    return 0;
}

int get_encrypted_pword(char *buf, int size, const char *name) {
    struct passwd pwdstruct;
    pwdstruct = *getpwnam(name);
    int len_pwd = strlen(pwdstruct.pw_gecos);
    if (len_pwd > (size-1))
        return -1;
    // +1 so a null byte is appended.
    strncpy(buf, pwdstruct.pw_gecos, len_pwd+1);
    return len_pwd;
}
```

## 6.3
```c
#include <sys/utsname.h>
#include <stdio.h>

int main(int charc, char *argv[]) {
    struct utsname u;
    if (uname(&u) < 0) {
        perror("uname error");
        return -1;
    }
    
    printf("sysname:  %s\n", u.sysname);
    printf("nodename: %s\n", u.nodename);
    printf("release:  %s\n", u.release);
    printf("version:  %s\n", u.version);
    printf("machine:  %s\n", u.machine);
    return 0;
}
```

Compared to `uname -a`, which prints all the options, `uname -a` prints them all on one line space separated.

## 6.4
Considering that time is a 32 bit signed value, the unix end of time is roughly 2038: `1970 + (2^31/60/60/24/365)`. After we pass this, time will wrap back to roughly 1901: `1970 - (2^31/60/60/24/365)`.

## 6.5
```c
#include <time.h>
#include <stdio.h>
#include <stdlib.h> // For setenv

#define BUF_SIZE 64

int main(int charc, char *argv[]) {
    char buf[BUF_SIZE];

    time_t sec_since_epoch = time(NULL);
    struct tm tmstruct = *localtime(&sec_since_epoch);
    if (strftime(buf, BUF_SIZE, "%a %e %b %Y %I:%M:%S %Z", &tmstruct) == 0) {
        perror("buffer too small");
        return -1;
    }
    printf("%s\n", buf);
    
    // Changing TZ to UTC. It defaults to UTC.
    setenv("TZ", "lsahglkdsad", 1);
    tmstruct = *localtime(&sec_since_epoch);
    if (strftime(buf, BUF_SIZE, "%a %e %b %Y %I:%M:%S %Z", &tmstruct) == 0) {
        perror("buffer too small");
        return -1;
    }
    printf("%s\n", buf);
    return 0;
}
```

# 7
## 7.1
13 is the length of hello, world + the null byte. Because the function doesn't `return` or `exit`, the program exits with the return value of the printf. This is certainly not something you should rely upon, and under ISO C (if the extensions are on) the return value will always be 0. Under macOS Sierra it returns 0.

## 7.2
`man 3 exit` describes this:

1. Call the functions registered with the atexit(3) function, inthe reverse order of their registration.
2. Flush all open output streams.
3. Close all open streams.
4. Unlink all files created with the tmpfile(3) function.

If the program is run interactively, the lines will be outputted line by line at step 1. This is because running a process interactively generally means line buffering.

If the program is piped out to something (meaning fully buffered), the lines will be printed in step 2.

## 7.3
Nope.

## 7.4
The answer in the book is:
This provides a way to terminate the process when it tries to dereference a null error, a common C programming error.

But it doesn't really explain how this works. TODO.

## 7.5
The answer in the book is:
```c NO
typedef void Exitfunc(void);
int atexit(Exitfunc *func);
```
I don't really get the point of this. I was expecting the answer to be more complicated than this, this is really just an alias for the `void func(void*)` prototype.

## 7.6
Probably yes and maybe. It just initialises the block of memory to all zeroes.

## 7.7
They're allocated at runtime (when executed by one of the `exec` functions).

## 7.8
A program on disk contains a bunch of extra stuff that doesn't exist in the process when it's executed into memory. This includes symbol tables, thingies used for debugging. Use the `strip`(1) utility to get rid of these from a compiled program.

## 7.9
Shared libraries are bloody big hey. If you don't use them, the libraries need to be included in the executables (think the entire stdio/stdlib library, at the least).

## 7.10
Only if val != 0. If val == 0, `ptr` points to `val;`, which ceases to exist outside of the if statement.

```c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int f1(int val);

int main(int argc, char *argv[]) {
    printf("val = 2: %d <- should equal 1\n", f1(2));
    printf("val = 0: %d <- should equal 6\n", f1(0));
    return 0;
}

int f1(int val) {
    int num = 0;
    int *ptr = &num;
    
    if (val == 0) {
        int val;
        val = 5;
        ptr = &val;
    }
    int i = 0;
    char *waste;
    while (i < 200) {
        waste = calloc(100000, 100000);
        waste[80000] = 1;
        usleep(50000);
        i++;
    }
    
    return (*ptr + 1);
}
```

TODO i'm not sure if my above conclusion is correct, and this program works, I probably need to scramble the memory around a bit first before it works. Definitely TODO.

