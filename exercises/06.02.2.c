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
