#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MYKEY 5

int main(int argc, char *argv[]) {
    int  qid; // message queue id
    int  i;
    char mymsg[64];
    char myreceived[64];

    unsigned long long_bytes = sizeof(long) / 8;

    for (i = 0; i < 5; i++) {
        // Shouldn't need IPC_EXCL, there shouldn't be a queue 
        // that already exists at this key.
        if ((qid = msgget(MYKEY, IPC_CREAT)) == -1) {
            perror("msgget error");
            return -1;
        }
        printf("qid: %d\n", qid);
        msgctl(qid, IPC_RMID, NULL);
    }

    for (i = 0; i < 5; i++) {
        // Shouldn't need IPC_EXCL, there shouldn't be a queue 
        // that already exists at this key.
        if ((qid = msgget(IPC_PRIVATE, IPC_CREAT|0200)) == -1) {
            perror("msgget error");
            return -1;
        }
        // First copy in the message type (we choose this and interpret
        // it however we want, 4 is just a random choice here).
        // See chapter 15.7 for another way to assemble the message (with a struct).
        // Also see memcpy_test.c in other for some experimentation about this.
        long code = 4;
        memcpy(mymsg, &code, long_bytes);
        memcpy(mymsg+long_bytes, "hello!\0", 7);
        if (msgsnd(qid, &mymsg, long_bytes+7, 0) == -1) {
            perror("msgsnd error");
            return -1;
        }
        // Not specified in the question, but we receive the message too just
        // to see that the msgsnd worked.
        if (msgrcv(qid, &mymsg, 64, 0, 0) == -1) {
            perror("msgsnd error");
            return -1;
        }
        // Note that the message code isn't copied into mymsg here.
        printf("Received message: %s\n", myreceived);
    }
    return 0;
}
