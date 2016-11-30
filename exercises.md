This should be readable as markdown. In sublime you should turn on C 
syntax highlighting though.

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

