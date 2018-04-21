//
// Created by tzdybel on 21.04.18.
//

#define INIT 1
#define MIRROR 2
#define CALC 3
#define TIME 4
#define END 5
#define REPLY 6
#define UNKNOWN 7

#define MAX_LENGTH 57

struct msgbuf {
    long mtype;
    char mtext[50];
    pid_t pid;
    int id;
};