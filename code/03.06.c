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
