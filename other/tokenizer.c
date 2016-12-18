/* Tokenizes input by delimiter and newline until EOF */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_TOKENS_START 4
#define MAX_TOKEN_LEN 64

int tokenize_stream(FILE *fp, char *tokens[], int len, char delimiter);

int main(int argc, char *argv[]) {  
    char delimiter = ' ';
    char *tokens[NUM_TOKENS_START];

    int num_tokens = tokenize_stream(stdin, tokens, NUM_TOKENS_START, delimiter);
    if (num_tokens < 0) {
        perror("tokenize_stream fail");
        return -1;
    }

    for (int i = 0; i < num_tokens; i++) {
        printf("%d: %s\n", i, tokens[i]);
    }

    return 0;
}

// On failure returns -1, on success returns the number of tokens read.
// This handles tokens up to length MAX_TOKEN_LEN, but could be adapted to have
// variable length tokens fairly easily.
int tokenize_stream(FILE *fp, char *tokens[], int len, char delimiter) {
    char newline = '\n';

    char buf[MAX_TOKEN_LEN];
    int buf_pos = 0;
    int num_tokens = 0;
    char c;
    char future;
    int almost_end = 0;
    while ((c = fgetc(fp)) != EOF) {

        if (buf_pos > (MAX_TOKEN_LEN - 2)) {
            perror("token too long");
            return -1;
        }

        // This lets us capture the last word properly.
        future = fgetc(fp);
        if (future == EOF) {
            if (c != delimiter && c != newline) {
                buf[buf_pos] = c;
                buf_pos += 1;
            }
            almost_end = 1;
        } else {
            // ungetc is only guaranteed to work for rewinding one char.
            ungetc(future, fp);
        }

        if (c == delimiter || c == newline || almost_end) {
            // Catch when we have a bunch of spaces after each other.
            if (buf_pos > 0) {
                buf[buf_pos] = '\0';
                // +1 for the null terminating byte.
                tokens[num_tokens] = malloc(buf_pos+1);
                if (tokens[num_tokens] == NULL) {
                    perror("malloc error");
                    return -1;
                }
                if (strncpy(tokens[num_tokens], buf, buf_pos+1) < 0) {
                    perror("strncpy error");
                    return -1;
                }
                buf_pos = 0;
                num_tokens += 1;
            }
        } else {
            buf[buf_pos] = c;
            buf_pos += 1;
        }

        if (num_tokens == len) {
            len *= 2;
            printf("Growing array for tokens.\n");
            *tokens = realloc(*tokens, len * sizeof(char*));
            if (tokens == NULL) {
                perror("realloc error");
                return -1;
            }
        }
    }
    // A return of EOF can mean EOF or error.
    if (ferror(fp) != 0) {
        perror("fgetc read error");
        return -1;
    }
    return num_tokens;
}