#include <stdio.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <signal.h>
#include <zconf.h>
#include "msgbuf.h"

void sigReact (int signum) {
    printf("Ending process...\n");
    exit(0);
}

void setSignals() {
    sigset_t sigset;
    sigfillset(&sigset);
    sigdelset(&sigset, SIGINT);
    sigprocmask(SIG_BLOCK, &sigset, NULL);

    signal(SIGINT, sigReact);
}

void mirror(struct msgbuf *msg) {
    int len = (int)strlen(msg->mtext);
    for (int i = 0; i < len/2; ++i)
    {
        char c = msg->mtext[i];
        msg->mtext[i] = msg->mtext[len-i-1];
        msg->mtext[len-i-1] = c;
    }
    msg->mtype = REPLY;
}

int numOfDigits(int x) {
    int i = 0;
    while (x > 0) {
        i++;
        x /= 10;
    }
    return i;
}

void calc(struct msgbuf *msg) {
    if(strncmp(msg->mtext, "CALC ", 5) != 0) {
        strcpy(msg->mtext, "Wrong format of command");
        return;
    }
    int x = (int)strtol(&msg->mtext[5], NULL, 10);
    int y = (int)strtol(&msg->mtext[5 + numOfDigits(x) + 1], NULL, 10);
    char sign = msg->mtext[5+numOfDigits(x)];
    int result = 0;
    switch (sign) {
        case '+':
            result = x+y;
            sprintf(msg->mtext, "%d", result);
            break;
        case '-':
            result = x-y;
            sprintf(msg->mtext, "%d", result);
            break;
        case '/':
            result = x/y;
            sprintf(msg->mtext, "%d", result);
            break;
        case '*':
            result = x*y;
            sprintf(msg->mtext, "%d", result);
            break;
        default:
            strcpy(msg->mtext, "Inappropriate sign");
            break;
    }
    msg->mtype = REPLY;
}

void askTime(struct msgbuf *msg) {
    time_t secs = time(NULL);
    char *t = ctime(&secs);
    strcpy(msg->mtext, t);
    msg->mtype = REPLY;
}

void initialize_client(int *clients, int nextID, struct msgbuf *msg) {
    clients[nextID] = (int)strtol(msg->mtext, NULL, 10);
    msg->mtype = REPLY;
    strcpy(msg->mtext, "Assigning ID");
    msg->id = nextID;
}

int main() {
    setSignals();

    int serverq;
    if ((serverq = msgget(ftok("test.txt", 3), 0666 | IPC_CREAT)) == -1) {
        printf("Error with client->server queue\n");
        exit(1);
    }
    int *clients = (int *)calloc(100, sizeof(int));
    int nextID = 0;
    struct msgbuf msg;

    struct msqid_ds stat;
    int continueWork = 1, timeToEnd = 0;
    sleep(10);
    while(continueWork > 0) {
        msgrcv(serverq, &msg, sizeof(struct msgbuf) - sizeof(long), 0, 0);
        switch(msg.mtype) {
            case INIT:
                initialize_client(clients, nextID, &msg);
                nextID++;
                break;
            case MIRROR:
                mirror(&msg);
                break;
            case CALC:
                calc(&msg);
                break;
            case TIME:
                askTime(&msg);
                break;
            case END:
                msgctl(serverq, IPC_STAT, &stat);
                continueWork = (int)stat.msg_qnum;
                timeToEnd = 1;
                continue;
            default:
                msg.mtype = REPLY;
                strcpy(msg.mtext, "Unknown command");
                break;
        }
        if (msgsnd(clients[msg.id], &msg, sizeof(struct msgbuf) - sizeof(long), 0) == -1) {
            printf("Sending failed\n");
        }
        if (timeToEnd) continueWork--;
    }
    printf("Closing server...\n");
    msgctl(serverq, IPC_RMID, NULL);
    return 0;
}