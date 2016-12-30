#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MYKEY 5
#define MSGLEN 32

unsigned long long_bytes = sizeof(long);

struct mymsgbuf {
    long mtype;
    char mtext[MSGLEN];    
};

int main(int argc, char *argv[]) {
    int  qid; // message queue id
    int  i;
    //char mymsg[64];
    //char myreceived[MSGLEN];

    struct mymsgbuf mymsgbuf;
    struct mymsgbuf bufrecv;

    printf("First loop\n");
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

    printf("Second loop\n");
    for (i = 0; i < 5; i++) {
        // Shouldn't need IPC_EXCL, there shouldn't be a queue 
        // that already exists at this key.
        if ((qid = msgget(IPC_PRIVATE, IPC_CREAT|0600)) == -1) {
            perror("msgget error");
            return -1;
        }
        // First copy in the message type (we choose this and interpret
        // it however we want, 4 is just a random choice here).
        // See chapter 15.7 for another way to assemble the message (with a struct).
        // Also see memcpy_test.c in other for some experimentation about this.
        long code = 4;
        /*
        memcpy(mymsg, &code, long_bytes);
        memcpy(mymsg+long_bytes, "hello!\0", 7);
        */

        // UPDATE: I'm questioning here whether msgsnd/msgrcv likes the way
        // I've assembled this, it may not be the valid struct that it wants.
        // See: https://stackoverflow.com/questions/2748995/c-struct-memory-layout
        // Instead we now just make the struct:
        mymsgbuf.mtype = code;
        strncpy(mymsgbuf.mtext, "hello!", MSGLEN);
        if (msgsnd(qid, &mymsgbuf, 7, 0) == -1) {
            perror("msgsnd error");
            return -1;
        }
        // Not specified in the question, but this code just receives the
        // message just to see that the msgsnd worked. To see the results
        // from ipcs, comment out this code.
        /*if (msgrcv(qid, &bufrecv, MSGLEN, 0, 0) == -1) {
            perror("msgsnd error");
            return -1;
        }
        // Note that the message code isn't copied into mymsg here.
        printf("Received message: %s\n", bufrecv.mtext);*/
    }
    return 0;
}
