#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define MY_BUF_SIZE 16

int main(int argc, char *argv[]) {
    char buf[MY_BUF_SIZE];

    // Open this very file. this_f is the file descriptor for it
    int this_f = open("current_file_offset.c", O_RDONLY);

    // Read from this file via the file descriptor. We read 
    // MY_BUF_SIZE number of bytes (chars)
    int bytes_read;
    if ((bytes_read = read(this_f, buf, MY_BUF_SIZE)) != MY_BUF_SIZE)
        perror("ruh roh\n");

    // Get the current file offset. Use SEEK_CUR with an offset of
    // 0 and lseek returns the current offset
    off_t offset = lseek(this_f, 0, SEEK_CUR);

    // Print the current file offset
    // lseek returns a type of off_t. TODO look up how you print these
    // Perhaps a call to sysconf or something to get the length of off_t?
    // For now we just cast to int. 
    printf("Current file offset: %d bytes\n", (int)offset);

    // We also print those 16 bytes we read to stdout for fun
    // STDOUT_FILENO is normally 1
    // Write the number of bytes read, which may be < MY_BUF_SIZE
    write(STDOUT_FILENO, buf, bytes_read);
    printf("\n");

    return 0;
}
