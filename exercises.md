# Exercises

For code you don't want compiled, do triple dash c NO:
`dash` `dash` `dash` `c` `[space]` `N` `O`.
See exercise 4.11 for an example.

You should have all of the source code from [here](http://www.apuebook.com/code3e.html) handy.

The word **TODO** is used to denote things I haven't done yet or wasn't sure about.

To include a file in the lib dir (e.g. error.c) do this:
//-#include "error.c"
Dirty I know but the alternative is to try and copy the makefiles that the 
original code was using.

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

#  4 File directories
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

#  5 Standard I/O Library

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

#  6 System Data Files and Information
## 6.1
Depends on the implementation. On Mac OS X you can't get to the encrypted password (the regular password file is shadowed automatically). On Linux there are a bunch of functions which can access the shadow password but you need superuser privileges to use them. The shadow password files aren't accessible by the world.

## 6.2
Unfortunately I'm on Mac OS X but this code should hopefully work on Linux.

```c NO
#include <shadow.h>
#include <string.h>
#include <stdio.h>

int get_encrypted_pword(char *buf, int size, const char *name);

int main(int charc, char *argv[]) {
    char buf[32];
    get_encrypted_pword(buf, 32, "daniel");
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

#  7 Process Environment
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

#  8 Process Control
TODO process accounting, I didn't really do that properly.

## 8.1
```c
#include "apue.h"

int globvar = 6;        /* external variable in initialized data */

int
main(void)
{
    int     var;        /* automatic variable on the stack */
    pid_t   pid;

    var = 88;
    printf("before vfork\n");   /* we don't flush stdio */
    if ((pid = vfork()) < 0) {
        perror("vfork error");
        return -1;
    } else if (pid == 0) {      /* child */
        globvar++;               /* modify parent's variables */
        var++;
        exit(0);               /* child terminates */ // NOTE
    }

    /* parent continues here */
    printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar,
      var);
    exit(0);
}
```

You'll notice that changing the exit on the line marked "NOTE" does not change the output. This is because, while the stdio libs flushes the buffer of the child upon the exit, it doesn't actually close the stream because the kernel does that anyway later. If the child *did* close the stream, the final printf wouldn't work because we closed stdout (remember the child and the parent share the file streams) with a vfork.

To simulate the behaviour of closing the streams, we can actually close the streams before calling exit, like so:

```c
#include "apue.h"

int globvar = 6;        /* external variable in initialized data */

int
main(void)
{
    int     var;        /* automatic variable on the stack */
    pid_t   pid;

    var = 88;
    printf("before vfork\n");   /* we don't flush stdio */
    if ((pid = vfork()) < 0) {
        perror("vfork error");
        return -1;
    } else if (pid == 0) {      /* child */
        globvar++;               /* modify parent's variables */
        var++;
        fclose(stdout);
        exit(0);               /* child terminates */ // NOTE
    }

    /* parent continues here */
    printf("pid = %ld, glob = %d, var = %d\n", (long)getpid(), globvar,
      var);
    exit(0);
}
```
You'll notice that the final printf doesn't print, of course. Keep in mind that it is the stdout stream that is closed; the STDOUT_FILENO file descriptor isn't closed. However some stdio libraries do also close the fd, in which case you need to `dup` before calling `fclose`.

## 8.2
I imagine vfork will occupy the stack frame of the function that calls it? According to the manpage however:
> vfork() can normally be used just like fork.  It does not work, however, to return while running in the childs context from the procedure that called vfork() since the eventual return from vfork() would then return to a no longer existent stack frame.

```c
// vforking from a chile
#include <unistd.h>
#include <stdio.h>

void myfunc();

int main(int charc, char *argv[]) {
    if ((setvbuf(stdout, NULL, _IONBF, 0)) != 0) {
        perror("setvbuf error");
        return -1;
    }
    printf("main here!\n");
    myfunc();
    printf("back to main!\n");
    return 2;
}

void myfunc() {
    pid_t pid;
    
    if ((pid = vfork()) < 0) {
        perror("vfork error");
        return;
    } else
    if (pid == 0) { // Child
        printf("sup dog it's the child after vfork\n");
        // If you don't have something like
        // _exit(0);
        // here, you're going to get a segfault.
    } else {        // Parent
        printf("parent after vfork!\n");
    }
    // At this point both the parent and child will try to implicitly return.
    // You could also have an explicit return, it would still fuck up.
}
```

If you run this and then try to check the output, you'll see the return is neither 1 or 2, but something like 139 or whatever the segfault code is:

    ./exercises/08.02
    "$?"
    -bash: 139: command not found

The problem is that after the child returns, the stack pops back to main and the child's version of main can then do whatever the hell it wants to do. If it had another function to do next, this other function could then change the memory of main if it wants (e.g. making a buffer of zeroes), it could do anything. The return information from a call is often stored in the stack frame for that function. So when the parent then returns from myfunc, the return information might've been overwritten by the child. This is a pretty poor explanation, the explanation in answer in the book is much better. TODO go read that.

## 8.3
```c
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
	

```
wait1 prints the following

> normal termination, exit status = 7
> abnormal termination, signal number = 6 (core file generated)
> abnormal termination, signal number = 8 (core file generated)

My program should print the same kind of information.

## 8.4
This happens because we have the parent write first, then the child. After the parent has written, it doesn't wait for the child to do its thing, it just terminates. As such, the next call can occur before the child finishes writing. If we had the child write first, with the parent waiting for it, this wouldn't be a problem, because the next call of `./tellwait1` only occurs once the parent exits.

To correct this we either need to let the child write first (i.e. be the one to do whatever first, with the parent waiting for it) or we need to make the parent wait for the child as well.

## 8.6
```c
//-#include "error.c"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "apue.h"

int main(int charc, char * argv[]) {
	
	pid_t pid;
	char buf[20];
	
	if ((pid = fork()) < 0) {
		err_sys("fork");
	} else
	if (pid == 0) {
		printf("this is the child, trying to exit!\n");
		exit(EXIT_SUCCESS);
	} else {
		printf("this is the parent, not waiting for the child\n");
		sprintf(buf, "ps %ld", (long int)pid);
		system(buf);
	}
	return 0;
}
```

When you run this program, you'll see something like this from ps:
    
      PID TTY      STAT   TIME COMMAND
    22909 pts/1    Z+     0:00 [08.06] <defunct>

This indicates that the process is a zombie. At the time of the ps call the parent hasn't terminated, so the zombie is still the child of the original parent. After the parent ends, if we do `ps <pid>` where <pid> is the pid printed from the first ps, we'll see that it's gone. That's because the zombie was inherited by init and then had wait called for it.

## 8.7

The question says to peek at the DIR struct for my implementation, but it very much seems
like you can't do that:

> This is the data type of directory stream objects.
> The actual structure is opaque to users.

Instead I've pulled the fd from the DIR struct and called fcntl on it.

```c
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
```

As expected, the flag is set when using the stdlib and not when using system calls.

#  9 Process Relationships

## 9.1

When a user logs out, the `login` process exits and the `init` process detects this (with `wait`). Upon detecting this, it writes the output to the wtmp file.

WRONG. Yes init detects when the login shell dies, but by catching the SIGCHLD signal.

For network logins, init doesn't play a part in this. Instead the networking daemon does it, e.g. `telnetd`.

## 9.2

I've already written something like this in session_ids.c; here is a modified version displaying the id of the controlling terminal (or lack thereof).

```c
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

/* This program demonstrates that the pid, pgid and "sid" of
** the leader of a new session are all the same */

int main(int argc, char *argv[]) {
	pid_t pid;

	if ((pid = fork()) < 0) {
		perror("fork error");
		return -1;
	} else
	if (pid == 0) {
		// Wait for the parent to die.
		while (getppid() != 1) {
			printf("Waiting for parent to die.\n");
			sleep(1);
		}

		pid = setsid();

		printf("pid, pgid and \"sid\" should be the same:\n");
		printf("pid: %d pgid: %d sid: %d\n", getpid(), getpgrp(), getsid(0));

		if ((open("/dev/tty", O_RDWR)) < 0) {
			printf("Child has no controlling terminal!\n");
		} else {
			printf("Child has a controlling terminal!\n");
		}

	} else {
		if ((open("/dev/tty", O_RDWR)) < 0) {
			printf("Parent has no controlling terminal!\n");
		} else {
			printf("Parent still has a controlling terminal!\n");
		}
		_exit(0);
	}
	return 0;
}
```

#  10 Signals

## 10.1
```c
#include "apue.h"
//-#include "error.c"

static void	sig_usr(int);	/* one handler for both signals */

int
main(void)
{
	if (signal(SIGUSR1, sig_usr) == SIG_ERR)
		err_sys("can't catch SIGUSR1");
	if (signal(SIGUSR2, sig_usr) == SIG_ERR)
		err_sys("can't catch SIGUSR2");
	// There used to be a for( ; ; ) here.
		pause();
}

static void
sig_usr(int signo)		/* argument is signal number */
{
	if (signo == SIGUSR1)
		printf("received SIGUSR1\n");
	else if (signo == SIGUSR2)
		printf("received SIGUSR2\n");
	else
		err_dump("received signal %d\n", signo);
}
```

Without the `for( ; ; )`, instead of continually catching the two signals (`SIGUSR1` AND `SIGUSR2`), it pauses and catches a signal only once and then terminates.

## 10.2

```c
#include <signal.h>
#include <stdio.h>
#include <string.h>

int sig2str(int signo, char *str);

int main(int argc, char *argv[]) {
    char buf[32];
    sig2str(5, buf);
    printf("sig 5: %s\n", buf);
    return 0;
}


int sig2str(int signo, char *str) {
    if (signo < 0 || signo > NSIG) {
        return -1;
    }
    
    // It's up to the caller to make sure there is
    // enough room in str for the return of strsignal
    // (plus a null byte \0).
    strcpy(str, strsignal(signo));
    
    return 0;
}
```

## 10.3

main -> sleep -> sig_int -> sig_alrm
           |                    |
            --------------------

When the sig_alarm comes in, the long_jmp just jumps straight back to sleep, essentially aborting/skipping sig_int entirely.

## 10.4
Race condition. If the alarm fires before we ever get to the set_jmp we're going to have problems. This could happen if the system is under heavy load so the alarm is called, then other processes are all serviced for a duration longer than the alarm. If the set_jmp hasn't been called, then when long_jmp is called in the sig_alrm it's going to have nowhere to jump so behaviour will be undefined.

## 10.5

TODO. The plan is to have a queue of timers. When a timer pops, you iterate through the queue (in the sig_alrm signal handler) and decrement each timer's value in the queue by the amount that just elapsed.

```c NO
#include <sys/time.h>
#include <stdio.h>

#define MAX_ALARMS 32

int queue[MAX_ALARMS];
int latest;

void pop();
int set_timer(int msecs);

int main(int argc, char *argv[]) {
    
    struct itimerval *value;
    getitimer(ITIMER_REAL, value);
    printf("hey\n");
    printf("curr: %6ld\n", value->it_value.tv_sec);
    
    return 0;
}

// Takes milliseconds.
int set_timer(int msecs) {
    latest = msecs;
    
    pop();
    
    return 0;
}
    
void pop() {

}
```

## 10.6
```c
// Remember, this line below will check ../lib for such a file.
#include "apue.h"
//-#include "tellwait.c"
//-#include "error.c"

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

void sig_int(int arg);

FILE *tmp;

// If you had this, it would also not be shared.
// You have to do something special to share vars like this.
//int counter = 1;

static volatile sig_atomic_t sigflag; /* set nonzero by sig handler */ 
static sigset_t newmask, oldmask, zeromask; 
static void sig_usr (int signo) { sigflag = 1; }

int main(int argc, char *argv[]) {
    
    // For some reason the tmpfile isn't working.
    
    if ((tmp = tmpfile()) == NULL) {
        err_sys("tmpfile error");
    }

    //if ((tmp = fopen("tmpfilebois", "w+")) == NULL)
    //    err_sys("fopen error");

    printf("value of close on exec flag: %d (sanity check)\n", fcntl(fileno(tmp), F_GETFD));

    putc('0', tmp);
    putc('\n', tmp);
    fflush(tmp); 
    // Without this fflush, the 0 might still be in the buffer after the fork so
    // it might get written to tmp twice.

    int counter = 1;
    
    pid_t pid;

    TELL_WAIT();
    
    if ((pid = fork()) < 0) {
        err_sys("fork error");
    } else
    if (pid == 0) {

        // Don't put the signal catching code in the child, let it die.

        for (;;) {
            // Make the child write first yeah?
            // If the parent writes first the child might not be waiting yet?
            WAIT_PARENT();
            printf("child writing %d\n", counter);
            fprintf(tmp, "c%d\n", counter);
            fflush(tmp);
            counter++;
            sleep(1);
            TELL_PARENT(getppid());
        }  
    } else {

        // Only put the signal catching code in the parent, so the handler
        // is only run once and the stuff is only printed out once.

        struct sigaction sig_int_act;

        sig_int_act.sa_handler = sig_int;
        sigemptyset(&sig_int_act.sa_mask);
        sig_int_act.sa_flags = 0;

        if (sigaction(SIGINT, &sig_int_act, NULL) < 0) {
            err_sys("sigaction SIGINT error");
        }

        TELL_CHILD(pid); // To get past the first wait in the child.
        for (;;) {
            // Make the child write first yeah?
            // If the parent writes first the child might not be waiting yet?
            WAIT_CHILD();
            printf("parent writing %d\n", counter);
            fprintf(tmp, "p%d\n", counter);
            fflush(tmp);
            counter++;
            sleep(1);
            TELL_CHILD(pid);
        }
    }
    return 0;
}

void sig_int(int arg) {
    fseek(tmp, 0, SEEK_SET);
    char c;
    while ((c = getc(tmp)) != EOF) {
        putchar(c);
    }

    struct sigaction dfl_act;

    dfl_act.sa_handler = SIG_DFL;
    sigemptyset(&dfl_act.sa_mask);
    dfl_act.sa_flags = 0;

    // Reset SIGINT to its dfl_act action and resend it.
    if (sigaction(SIGINT, &dfl_act, NULL) < 0) {
        err_sys("sigaction SIGINT error");
    }

    kill(getpid(), SIGINT);

}

```

## 10.7
By resetting the disposition and then killing it with SIGABRT for real, the 
termination status reflects that it was killed with SIGABRT. `_exit` wouldn't 
do this. TODO Why can't we just call `_exit` with the SIGBART termination 
status? The reason is that this isn't a cross platform way of doing it. On some 
systems, the exit status from a signal is something like 127 + SIGNAL_CODE. In 
the end writing something that duplicates what the shell does would not only be 
flaky and potentially difficult to make cross platform, but would end up being 
more code than just resetting the disposition and sending the signal again.

This code snippet demonstrates different return codes (check with `echo $?`):

```c
#include <signal.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    // Uncomment one of the two below to see the termination status of each.
    //_exit(SIGABRT);
    //kill(getpid(), SIGABRT);
    return 0; // Shouldn't get here.
}
```

## 10.8
The effective user id will either be the owner of the receiving process or root,
so it doesn't tell us much. The real user id is obviously then more informative.

## 10.9
```c
#include "apue.h"
#include <signal.h>
#include <stdio.h>
//-#include "error.c"

int main(int argc, char *argv[]) {
    sigset_t sigset;

    if (sigprocmask(0, NULL, &sigset) < 0) {
        err_sys("sigprocmask error");
    }

    for (int i = 0; i < NSIG; i++) {
        if (sigismember(&sigset, i))
            printf("%s ", sys_siglist[i]);
    }
    printf("\n");
    return 0;
}
```

This doesn't seem to work, but I think the mask is just unpopulated?

## 10.10
Time drifts after successive `sleep` calls. cron calls sleep every 60 seconds as
well to schedule the next 60 seconds, but occasionally resyncs time with a 
`sleep(59)`.

## 10.11

The `signal_intr` function is pretty much just a wrapper around the `sigaction` function.
It creates a signal handler in which no signals are blocked. As such, it can be interrupted
by any other incoming signal (except for the signal for said handler, i.e. the handler for SIGINT won't be interupted by another SIGINT). If the system defines it, the SA_INTERRUPT flag disables interrupts while the handler is running:

> When set, this indicates a "fast" interrupt handler. Fast handlers are executed with interrupts disabled on the current processor. From [here.](http://www.makelinux.net/ldd3/chp-10-sect-2)

The SA_INTERRUPT flag shouldn't be used unless you really need it. It's better to define an
sa_mask which explicitly blocks flags you don't want the handler to deal with.

This code enters the SIGALRM handler and just stays there. You then see that you can interrupt
it with a SIGINT for example.

Remember, the sa_mask part of the sigaction struct defines which signal are blocked. An
empty mask means no signals are blocked.

```c
#include "apue.h"
//-#include "signalintr.c"
#include <unistd.h>

void my_handler(int arg);

int main(int argc, char *argv[]) {
	signal_intr(SIGALRM, my_handler);
	alarm(1);
	sleep(3);
	return 0;
}

void my_handler(int arg) {
	int i = 0;
	for(;;) {
		printf("%d in signal handler %d\n", i++, arg);
		sleep(1);
		alarm(1); // This won't do anything.
		if (i > 5) {
			kill(getpid(), SIGINT); // This will.
		}
	}
}
```

The function returns the old handler function.

====================================

```c
// From fileio/mycat.c
#include "apue.h"
#include <signal.h>
#include <sys/resource.h>
//-#include "signalintr.c"
//-#include "error.c"

#define	BUFFSIZE	100

void my_handler(int arg);

int
main(void)
{
	int		n;
	int     written;
	char	buf[BUFFSIZE];

	struct rlimit my_limit;
	my_limit.rlim_cur = 2048;
	my_limit.rlim_max = 10000;
	setrlimit(RLIMIT_FSIZE, &my_limit);

	// SIGXFSZ File size limit exceeded (4.2BSD)
	signal_intr(SIGXFSZ, my_handler);

	while ((n = read(STDIN_FILENO, buf, BUFFSIZE)) > 0) {
		if ((written = write(STDOUT_FILENO, buf, n)) != n) {
		    fprintf(stderr, "Wrote %d bytes\n", written);
			err_sys("write error");
		}
	}

	if (n < 0)
		err_sys("read error");

	exit(0);
}

// arg is the signal value, e.g. 14 for SIGALRM.
void my_handler(int arg) {
	printf("Caught %d, %s\n", arg, strsignal(arg));
}

```

Run `ulimit -f 1024` to set the RLIMIT_FSIZE resource limit. Note that you can't 
raise it again without superuser privileges. See [here](https://stackoverflow.com/questions/17483723/command-not-found-when-using-sudo-ulimit) 
if you have difficulties with that. Note that this value is the number of 
blocks, not the number of bytes. `setrlimit` is used in the program because 
it does it in bytes, which is much clearer.

Now if you make yourself a random 2mb file and run the program:

    dd if=/dev/zero of=2mbfile  bs=2M  count=1
    cat 2mbfile | ./code/10.11.2 > test

You'll get something like this:
    
    Wrote 76 bytes
    write error: Success

But why is it only 76 bytes instead of 1024? You'll also notice that if you do 
`ulimit -f 512` instead of getting something like:
    
    Wrote 38 bytes
    write error: Success

You get:
    
    Wrote 88 bytes
    write error: Success
 
This is because the "Wrote n bytes" text is reporting how many bytes the latest 
call to writesuccessfully wrote. Remember that write fills up to the buffer 
only, so it takes multiple calls to write since we designated such a small 
buffer (100 bytes).

Why 24? Because the buffer is 100 bytes. As such, the first 1000 bytes copy in 10 writes no
worries, then the final copies only 24 bytes.

## 10.12

You will have to run this as superuser for the setrlimit to work.

```c
#include "apue.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
//-#include "signalintr.c"

// 1gb
#define BUFSIZE 1000000000
char buf[BUFSIZE];
// You have to put this here (this is the data segment).
// If you put it in main, it'll be on the stack. If you check
// ulimit -a, you'll see that it is by default not large enough
// for a buffer of 1gb.

void my_handler(int arg);

int main(int argc, char *argv[]) {

	struct rlimit my_limit;
	my_limit.rlim_cur = 1000000000;
	my_limit.rlim_max = 1000000000;
	if (setrlimit(RLIMIT_FSIZE, &my_limit) < 0) {
		perror("setrlimit");
		return -1;
	}

	signal_intr(SIGALRM, my_handler);

	FILE *f = fopen("blah", "wb+");

	alarm(2); // Make sure this is shorter than how long the fwrite takes

	int written = fwrite(buf, 1, BUFSIZE, f);

	if (ferror(f))
		printf("ferror set, there was an error\n");
	else
		printf("ferror not set, disregard the errno below\n");
	printf("Wrote %d bytes, errno = %d msg: %s\n", written, errno, strerror(errno));

}

void my_handler(int arg) {
	printf("Caught %d: %s\n", arg, strsignal(arg));
}
```

This displays pretty concering behaviour here. While we're busy writing our god awfully
large 1gb file of junk, the OS blocks the signal from getting to the process until the
`fwrite` completes. This is verified to be happening, it's not a race from the alarm firing
before we get to the `fwrite` call.

#  11 Threads

## 11.1
The secret is to create the foo struct with malloc so the memory persists
after the function exits (contrary to automatic variables, whose memory gets
released when the function exits).

```c
#include "apue.h"
#include <pthread.h>
//-#include "error.c"

struct foo {
        int a, b, c, d;
};

void
printfoo(const char *s, const struct foo *fp)
{
        printf("%s", s);
        printf("  structure at 0x%lx\n", (unsigned long)fp);
        printf("  foo.a = %d\n", fp->a);
        printf("  foo.b = %d\n", fp->b);
        printf("  foo.c = %d\n", fp->c);
        printf("  foo.d = %d\n", fp->d);
}

void *
thr_fn1(void *arg)
{
        struct foo *myfoo;
        if ((myfoo = malloc(sizeof(struct foo))) == NULL)
            err_sys("malloc error");
        myfoo-> a = 1;
        myfoo-> b = 2;
        myfoo-> c = 3;
        myfoo-> d = 4;
        printfoo("thread 1:\n", myfoo);
        pthread_exit((void *)myfoo);
}

void *
thr_fn2(void *arg)
{
        printf("thread 2: ID is %lu\n", (unsigned long)pthread_self());
        pthread_exit((void *)0);
}

int
main(void)
{
        int                     err;
        pthread_t       tid1, tid2;
        struct foo      *fp;

        err = pthread_create(&tid1, NULL, thr_fn1, NULL);
        if (err != 0)
                err_exit(err, "can't create thread 1");
        err = pthread_join(tid1, (void *)&fp);
        if (err != 0)
                err_exit(err, "can't join with thread 1");
        sleep(1);
        printf("parent starting second thread\n");
        err = pthread_create(&tid2, NULL, thr_fn2, NULL);
        if (err != 0)
                err_exit(err, "can't create thread 2");
        sleep(1);
        printfoo("parent:\n", fp);
        exit(0);
}
```

## 11.2
Firstly, make sure you acquire a write lock on the queue.
Secondly, note that there is a window between when a thread finds a job with
`job_find` and when the job is removed with `job_remove`. In this window, the
master thread could change the id of a job, which will cause problems (namely
that the other thread will take the job still even though it no longer belongs
to them). The way around this is to include a reference count inside the job
struct. When a worker process finds a job with `job_find`, increment the count.
This way, when the master thread goes to change the job id, if the count is
greater than zero we know a worker is about to take it, so we can act
accordingly (most likely just let them take the job).

## 11.3
```c NO
#include <stdlib.h>
#include <pthread.h>

struct job {
	struct job *j_next;
	struct job *j_prev;
	pthread_t   j_id;   /* tells which thread handles this job */
	/* ... more stuff here ... */
};

struct queue {
	struct job      *q_head;
	struct job      *q_tail;
	pthread_rwlock_t q_lock;
};

/*
 * Initialize a queue.
 */
int
queue_init(struct queue *qp)
{
	int err;

	qp->q_head = NULL;
	qp->q_tail = NULL;
	err = pthread_rwlock_init(&qp->q_lock, NULL);
	if (err != 0)
		return(err);
	/* ... continue initialization ... */
	return(0);
}

/*
 * Insert a job at the head of the queue.
 */
void
job_insert(struct queue *qp, struct job *jp)
{
	pthread_rwlock_wrlock(&qp->q_lock);
	jp->j_next = qp->q_head;
	jp->j_prev = NULL;
	if (qp->q_head != NULL)
		qp->q_head->j_prev = jp;
	else
		qp->q_tail = jp;	/* list was empty */
	qp->q_head = jp;
	pthread_rwlock_unlock(&qp->q_lock);
}

/*
 * Append a job on the tail of the queue.
 */
void
job_append(struct queue *qp, struct job *jp)
{
	pthread_rwlock_wrlock(&qp->q_lock);
	jp->j_next = NULL;
	jp->j_prev = qp->q_tail;
	if (qp->q_tail != NULL)
		qp->q_tail->j_next = jp;
	else
		qp->q_head = jp;	/* list was empty */
	qp->q_tail = jp;
	pthread_rwlock_unlock(&qp->q_lock);
}

/*
 * Remove the given job from a queue.
 */
void
job_remove(struct queue *qp, struct job *jp)
{
	pthread_rwlock_wrlock(&qp->q_lock);
	if (jp == qp->q_head) {
		qp->q_head = jp->j_next;
		if (qp->q_tail == jp)
			qp->q_tail = NULL;
		else
			jp->j_next->j_prev = jp->j_prev;
	} else if (jp == qp->q_tail) {
		qp->q_tail = jp->j_prev;
		jp->j_prev->j_next = jp->j_next;
	} else {
		jp->j_prev->j_next = jp->j_next;
		jp->j_next->j_prev = jp->j_prev;
	}
	pthread_rwlock_unlock(&qp->q_lock);
}

/*
 * Find a job for the given thread ID.
 */
struct job *
job_find(struct queue *qp, pthread_t id)
{
	struct job *jp;

	if (pthread_rwlock_rdlock(&qp->q_lock) != 0)
		return(NULL);

	for (jp = qp->q_head; jp != NULL; jp = jp->j_next)
		if (pthread_equal(jp->j_id, id))
			break;

	pthread_rwlock_unlock(&qp->q_lock);
	return(jp);
}
```

STILL TODO

## 11.4
The first option is safer. It might result in some threads waiting for a bit
while we wait for the first thread to hit step 4, but it means that a thread
will grab it straight up when it's available without messing anything up.

With the second option, in between steps 3 and 4 the condition can be altered
by another thread, which means when the first thread calls step 4 the condition
will no longer be valid. This means that the awakened threads must check the
condition again to make sure that it is what we want (to be true).

## 11.5
TODO

#  12 Thread Control

## 12.1
The output becomes fully buffered instead of line buffered. As such the lines 
to be printed from the original parent before the `fork()` are still in the 
buffers before the fork. A few tactically placed `fflush()`s would solve this, 
but the easiest fix is to just set line buffering to line buffered with `setvbuf`.

## 12.2
See my question about this question on stack overflow [here](https://stackoverflow.com/questions/41257768/thread-safe-reentrant-async-signal-safe-putenv).

```c
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

```

## 12.3
Just whacking an all-signal block at the start of your function and then 
removing it at the end doesn't guarantee that it is signal safe. The reason 
for this is some of the functions in between the block/unblock might unblock 
the signals themselves, making the code vulnerable.

## 12.4
TODO

## 12.5
If we want to fork-exec mainly, meaning that there is a totally different 
program, instead of tasks connected by a common thread (excuse the pun).

## 12.6
Signals don't play too nice with threads. Unless it's a signal that a thread
caused to be generated, in which case the signal will generally be delivered
to that thread, the signal will be sent to any random thread. Point being,
don't rely on signals in a multi-threaded environment like you would in a 
single-threaded environment.

Original code in figure 10.29 to change. Go see the answer, the code is totally
different. You can use `select`, which is thread safe, to implement this.
Beyond that there are no special tricks.

```c NO
#include "apue.h"
#include <signal.h>
#include <unistd.h>

static void
sig_alrm(int signo)
{
	/* nothing to do, just return to wake up the pause */
}

unsigned int
sleep(unsigned int seconds)
{
	struct sigaction newact, oldact;
	sigset_t         newmask, oldmask, suspmask;
	unsigned int     unslept;

	newact.sa_handler = sig_alrm;
	sigemptyset(&newact.sa_mask);
	newact.sa_flags = 0;
	sigaction(SIGALRM, &newact, &oldact);

	sigemptyset(&newmask);
	sigaddset(&newmask, SIGALRM);
	sigprocmask(SIG_BLOCK, &newmask, &oldmask);

	alarm(seconds);		/* start the timer */
	suspmask = oldmask;

	// Make sure SIGALRM isn't blocked.
	sigdelset(&suspmask, SIGALRM);

	// Wait for signal to be caught.
	sigsuspend(&suspmask);
	// Some signal has been cuaght, SIGALRM is now blocked.

	unslept = alarm(0);

	// Reset previous action
	sigaction(SIGALRM, &oldact, NULL);

	// Reset signal mask, which unlocks SIGALRM.
	sigprocmask(SIG_SETMASK, &oldmask, NULL);

	return(unslept);
}
```

## 12.7
We should be able to yeah, since after a fork the two processes have their own
memory spaces (technically not because of copy-on-write, but as far for all
intents and purposes we can consider them separate).

WRONG! The question wasn't clear about this but it is referring to atfork
handlers, in which case they're aren't entirely disjoint yet. Really, after a
`fork` be incredibly careful with everything you do before calling `exec`.

## 12.8
The calculating of times is super dirty. The `timeout` function takes an
absolute time, but we convert it into a relative amount of time left until
that absolute time. Instead, we could make the timeout function just take a 
relative time in the first place. Alternatively, we could set the flag in the
clock_nanosleep function from `0` to `TIMER_ABSTIME`, meaning it takes an
absolute time, which would also save us the conversion. The latter of these
options would probably be the simplest, but it's up to preference. Either are
better than what the code does currently (absolute -> relative).

#  13 Daemon Processes
## 13.1
Go read the manpage for `chroot`, it doesn't work quite how you think. Remember
that this is not changing the current working directory, but changing the root,
as the name of the function implies. As such, if I for example `chroot`ed to
`/home/daniel`, if I then opened `/`, that `/` refers to `/home/daniel`. Go
read the manpage though, there are a few quirks to how it works that you should
know about.

Unless the program is chrooting to the actual original root `/`, then you won't
be able to access `/dev/log`, because the daemon will no longer have access
to the true absolute root of the file system. The secret is to call `openlog`
with the LOG_NDELAY option set before calling `chroot`. Even after the call to
`chroot`, the file descriptor to the log socket will still be valid.

## 13.2
I have a Unix & Linux stack exchange question about this here:
https://unix.stackexchange.com/questions/332085/rsyslogd-session-leader

## 13.3

- **systemd**:		"A great kernel to run alongside Linux".
					Controversially replaces init, is heavy and far reaching.
- **kthreadd**: 	Used by the kernel to spawn more kernel space threads.
- **ksoftirqd**:	One of these daemons exists for each cpu-core. You'll see
					on this system that there are 8 of these, up to 
					ksoftirqd/7 (from ksoftirqd/0). This is because this cpu,
					the 6700k, has 8 virtual cpus (4 cores, 2 threads each).
					The process handles soft interrupts.
- **watchdog**:		See [here.](https://linux.die.net/man/8/watchdog)
- **migration**:	Distributes workload accross cpu cores. There should be
					one per core, like ksoftirqd.
- **rcu_sched**:	Also **rcu_bh**. See [here](https://lwn.net/Articles/262464/).
- **bioset**:		These are to do with block IO. See [here.](https://askubuntu.com/questions/673466/bioset-process-what-is-its-function/687660)

There are no doubt way more of these, such as to do with networking.

I'm not sure if `kworker` is really a daemon per se, considering that they are
spawned just to do work for the kernel rather than living for the whole system
up-time, but [here](https://askubuntu.com/questions/33640/kworker-what-is-it-and-why-is-it-hogging-so-much-cpu)) 
is info on them regardless.

## 13.4
```c
#include "apue.h"
#include <unistd.h>
#include <fcntl.h>

//-#include "daemonize.c"
//-#include "error.c"

#define OUTFILE "/home/daniel/myout.txt"
#define BUFSIZE 32

int main(int argc, char *argv[]) {
	int fd;
	char lname[BUFSIZE];

	daemonize(argv[0]);

	if ((fd = open(OUTFILE, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP)) < 0) {
		perror("open error");
		return -1;
	}

	if (getlogin_r(lname, BUFSIZE) != 0) {
		perror("getlogin error");
		return -1;		
	}

	if (write(fd, lname, strlen(lname)) < 0) {
		perror("write error");
		return -1;			
	}

	return 0;
}
```

For some reason I couldn't get this function to work when using the stdio lib.
This is probably something worth looking into TODO.

Note that these `perrors` are essentially useless because stderr is closed.
It would be better to use the syslog function, since it has been daemonized
and all, especially considering that the `daemonize` function set it up for us
with the call to `openlog`.

#  14 Advanced I/O
## 14.1
Need 3 processes. One process read locks, the second read locks, the first drops it, then reaquires it, then the second drops then reaquires, etc. Then third meanwhile tries to get a write lock.

An easier way to test this is if a read lock is held, then a write lock waits for the lock, if you can queue another read lock while the write lock is waiting then you'll have the problematic behaviour of never being able to acquire a write lock.

```c NO
TODO
```

## 14.2
See the answer and read the manpages, nothing too special.

## 14.3
It's complicated. See the answer, you need to do a bunch of stuff with `#define`s and symbols, as well as system configuration.

## 14.4
TODO

## 14.5
```c
#include <stdio.h>
#include <sys/select.h>

int sleep_us(int ms);

int main(int argc, char *argv[]) {
    printf("Waiting 2 seconds\n");
    sleep_us(2000000);
    return 0;
}

int sleep_us(int ms) {
    struct timeval tv;
    // This is better than just putting all the ms in tv_usec.
    tv.tv_sec  = ms / 1000000;
    tv.tv_usec = ms % 1000000;
    // Will return 0 on success since all the sets are NULL.
    return select(0, NULL, NULL, NULL, &tv);
}
```

## 14.6
You would think yes, because advisory should work if you write your program to respect the locks, but unfortunately it doesn't work because locks are lost accross a fork. This means that a child can't start with any locks of its own, making it hard for a parent to wait for the child.

## 14.7
```c
#include "apue.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
//-#include "setfl.c"
//-#include "error.c"

int main(int argc, char *argv[]) {
    int fds[2];

    if (pipe(fds) < 0) {
        perror("pipe error");
        return -1;
    }

    set_fl(fds[1], O_NONBLOCK);

    int ret;
    int counter = 0;
    while ((ret = write(fds[1], "writing data friend", 19)) > 0) {
        counter += ret;
        // This fun little escape makes it reprint on the same line.
        printf("\033[A\33[2K\r%5d bytes written\n", counter);
    }

    printf("write failed with error %d: %s\n", errno, strerror(errno));
    return 0;
}
```

On my current system this gets to 65531 bytes (so 65536).

PIPE_BUF is only 4096 bytes on my system. The 65536 value is the maxmimum amount of data allowed in the pipe, while the 4096 value is the maximum amount of data that can be written atomically (in one go).

## 14.8
You need to compile with the `-lrt` gcc flag for the aio functions.

```c
#include "apue.h"
#include <ctype.h>
#include <fcntl.h>
#include <aio.h>
#include <errno.h>
//-#include "setfl.c"
//-#include "error.c"

#define BSZ 4096
#define NBUF 8

enum rwop {
    UNUSED = 0,
    READ_PENDING = 1,
    WRITE_PENDING = 2
};

struct buf {
    enum rwop     op;
    int           last;
    struct aiocb  aiocb;
    unsigned char data[BSZ];
};

struct buf bufs[NBUF];

unsigned char
translate(unsigned char c)
{
    /* same as before */
    if (isalpha(c)) {
        if (c >= 'n')
            c -= 13;
        else if (c >= 'a')
            c += 13;
        else if (c >= 'N')
            c -= 13;
        else
            c += 13;
    }
    return(c);
}

int
main(int argc, char* argv[])
{
    int                     ifd, ofd, i, j, n, err, numop;
    struct stat             sbuf;
    const struct aiocb      *aiolist[NBUF];
    off_t                   off = 0;
    // Set this to 1 if connected to a pipe or terminal.
    int                     is_pipe_or_tty = 0;

    if (argc != 1)
        err_quit("usage: %s", argv[0]);
    // These are the important lines.
    ifd = STDIN_FILENO;
    ofd = STDOUT_FILENO;
    if (fstat(ifd, &sbuf) < 0)
        err_sys("fstat failed");

    // Pipe or something like that, they don't have a size.
    if (sbuf.st_size == 0) {
        is_pipe_or_tty = 1; 
    }

    // Don't think this is necessary / does anything.
    //set_fl(STDIN_FILENO, O_ASYNC);
    //set_fl(STDOUT_FILENO, O_ASYNC);

    /* initialize the buffers */
    for (i = 0; i < NBUF; i++) {
        bufs[i].op = UNUSED;
        bufs[i].aiocb.aio_buf = bufs[i].data;
        bufs[i].aiocb.aio_sigevent.sigev_notify = SIGEV_NONE;
        aiolist[i] = NULL;
    }

    numop = 0;
    for (;;) {
        for (i = 0; i < NBUF; i++) {
            switch (bufs[i].op) {
            case UNUSED:
                /*
                 * Read from the input file if more data
                 * remains unread.
                 */
                if (off < sbuf.st_size || is_pipe_or_tty) {
                    bufs[i].op = READ_PENDING;
                    bufs[i].aiocb.aio_fildes = ifd;
                    bufs[i].aiocb.aio_offset = off;
                    off += BSZ;
                    // TODO how does this work with a pipe...
                    if (off >= sbuf.st_size) {
                        bufs[i].last = 1;
                        is_pipe_or_tty = 0; // THIS
                    }
                    bufs[i].aiocb.aio_nbytes = BSZ;
                    if (aio_read(&bufs[i].aiocb) < 0)
                        err_sys("aio_read failed");
                    aiolist[i] = &bufs[i].aiocb;
                    numop++;
                }
                break;

            case READ_PENDING:
                if ((err = aio_error(&bufs[i].aiocb)) == EINPROGRESS)
                    continue;
                if (err != 0) {
                    if (err == -1)
                        err_sys("aio_error failed");
                    else
                        err_exit(err, "read failed");
                }

                /*
                 * A read is complete; translate the buffer
                 * and write it.
                 */
                if ((n = aio_return(&bufs[i].aiocb)) < 0)
                    err_sys("aio_return failed");
                if (n != BSZ && !bufs[i].last)
                    err_quit("short read (%d/%d)", n, BSZ);
                for (j = 0; j < n; j++)
                    bufs[i].data[j] = translate(bufs[i].data[j]);
                bufs[i].op = WRITE_PENDING;
                bufs[i].aiocb.aio_fildes = ofd;
                bufs[i].aiocb.aio_nbytes = n;
                if (aio_write(&bufs[i].aiocb) < 0)
                    err_sys("aio_write failed");
                /* retain our spot in aiolist */
                break;

            case WRITE_PENDING:
                if ((err = aio_error(&bufs[i].aiocb)) == EINPROGRESS)
                    continue;
                if (err != 0) {
                    if (err == -1)
                        err_sys("aio_error failed");
                    else
                        err_exit(err, "write failed");
                }

                /*
                 * A write is complete; mark the buffer as unused.
                 */
                if ((n = aio_return(&bufs[i].aiocb)) < 0)
                    err_sys("aio_return failed");
                if (n != bufs[i].aiocb.aio_nbytes)
                    err_quit("short write (%d/%d)", n, BSZ);
                aiolist[i] = NULL;
                bufs[i].op = UNUSED;
                numop--;
                break;
            }
        }
        if (numop == 0) {
            if (off >= sbuf.st_size)
                break;
        } else {
            if (aio_suspend(aiolist, NBUF, NULL) < 0)
                err_sys("aio_suspend failed");
        }
    }

    bufs[0].aiocb.aio_fildes = ofd;
    if (aio_fsync(O_SYNC, &bufs[0].aiocb) < 0)
        err_sys("aio_fsync failed");
    exit(0);
}
```

I'm not sure if the above actually solves the problem. To really see what I'm
talking about, try this:

    echo hello | ./code/14.08 | ./code/14.08

You'll see that it gets converted by ROT13 and then converted again back to normal,
but after it's done it, it just lingers without actually terminating. Even with
just a single command like:
    
    echo hello | ./code/14.08

It takes quite a while for the program to die. Perhaps I should be dealing with an
EOF or something? Because redirecting from a file outputs it and stop instantly.

**Update**: Go to the point in the code marked **THIS**. After we hit the final read,
I set `is_pipe_or_tty` to `0`. Because we can't check against the size of the file,
we use this condition to tell the program to stop trying to read more, much as we 
used it to indicate that we should readnj in the first place.

## 14.9
```c
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/uio.h>

#define START_BYTES 25
#define END_BYTES 3000000

void timespec_diff(struct timespec *start, struct timespec *stop,
         struct timespec *result);

int main(int argc, char *argv[]) {
    struct timespec start, end, result;

    int fd = open("testbois", O_CREAT|O_TRUNC|O_WRONLY, 0777);
    int fdrand = open("/dev/random", O_RDONLY);

    for (int b = START_BYTES; b <= END_BYTES; b *= 2) {
        char buf1[b];
        char buf2[b];

        // This shouldn't really do anything.
        read(fdrand, buf1, b);
        read(fdrand, buf2, b);

        // Manual copy and write.
        clock_gettime(CLOCK_REALTIME, &start);

        char buf3[b*2];
        memcpy(buf3, buf1, b);
        memcpy(buf3+b, buf2+b, b);

        write(fd, buf3, b*2);

        clock_gettime(CLOCK_REALTIME, &end);
        timespec_diff(&start, &end, &result);
        printf("%7d copy and write: %lld.%.9ld", b, (long long)result.tv_sec, result.tv_nsec);

        ftruncate(fd, 0);

        // Using writev.
        read(fdrand, buf1, b);
        read(fdrand, buf2, b);

        clock_gettime(CLOCK_REALTIME, &start);

        struct iovec bufs[2];
        bufs[1].iov_base = buf1;
        bufs[1].iov_len  = b;
        bufs[2].iov_base = buf2;
        bufs[2].iov_len  = b;

        writev(fd, bufs, 2);

        clock_gettime(CLOCK_REALTIME, &end);
        timespec_diff(&start, &end, &result);
        printf(" writev: %lld.%.9ld\n", (long long)result.tv_sec, result.tv_nsec);

        ftruncate(fd, 0);
    }
    return 0;
}

// Thanks: https://gist.github.com/diabloneo/9619917
void timespec_diff(struct timespec *start, struct timespec *stop,
         struct timespec *result)
{
    if ((stop->tv_nsec - start->tv_nsec) < 0) {
        result->tv_sec = stop->tv_sec - start->tv_sec - 1;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
    } else {
        result->tv_sec = stop->tv_sec - start->tv_sec;
        result->tv_nsec = stop->tv_nsec - start->tv_nsec;
    }
}
```

The results from this seem counter intuitive, copy and write should be faster at
the start and then slower later. TODO what's up with this?

## 14.10
The program in question is `../advio/mcopy2`. It doesn't look like the access
time is updated under WSL. According to the book all four platforms update the
last access time under at least the default file system (indeed perhaps why WSL
doesn't update it, since it's Linux but the files are on a NTFS drive).

## 14.11
```c
#include "apue.h"
#include <fcntl.h>
#include <sys/mman.h>
//-#include "error.c"

#define COPYINCR (1024*1024*1024)       /* 1 GB */

int
main(int argc, char *argv[])
{
    int             fdin, fdout;
    void            *src, *dst;
    size_t          copysz;
    struct stat     sbuf;
    off_t           fsz = 0;

    if (argc != 3)
        err_quit("usage: %s <fromfile> <tofile>", argv[0]);

    if ((fdin = open(argv[1], O_RDONLY)) < 0)
        err_sys("can't open %s for reading", argv[1]);

    if ((fdout = open(argv[2], O_RDWR | O_CREAT | O_TRUNC,
            FILE_MODE)) < 0)
        err_sys("can't creat %s for writing", argv[2]);

    if (fstat(fdin, &sbuf) < 0)   /* need size of input file */
        err_sys("fstat error");

    if (ftruncate(fdout, sbuf.st_size) < 0) /* set output file size */
        err_sys("ftruncate error");

    while (fsz < sbuf.st_size) {
        if ((sbuf.st_size - fsz) > COPYINCR)
            copysz = COPYINCR;
        else
            copysz = sbuf.st_size - fsz;

        if ((src = mmap(0, copysz, PROT_READ, MAP_SHARED,
            fdin, fsz)) == MAP_FAILED)
                err_sys("mmap error for input");
        
        // Closing input file as per the question.
        if (close(fdin) < 0)
            err_sys("couldn't close fdin");

        if ((dst = mmap(0, copysz, PROT_READ | PROT_WRITE,
            MAP_SHARED, fdout, fsz)) == MAP_FAILED)
                err_sys("mmap error for output");

        memcpy(dst, src, copysz);       /* does the file copy */
        munmap(src, copysz);
        munmap(dst, copysz);
        fsz += copysz;
    }
    exit(0);
}
```

If you use this (just like you would use mcopy2), you'll see that it still works.