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
