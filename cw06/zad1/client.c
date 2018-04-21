//
// Created by tzdybel on 21.04.18.
//

#include <stdio.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <zconf.h>
#include <memory.h>
#include <signal.h>
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

void init(struct msgbuf *msg, int serverq, int clientq) {
    msg->mtype = INIT;
    msg->pid = getpid();
    sprintf(msg->mtext, "%d", clientq);
    msg->id = 0;

    if(msgsnd(serverq, msg, sizeof(struct msgbuf) - sizeof(long), 0) == -1) {
        printf("Sending failed\n");
        exit(1);
    }

    msgrcv(clientq, msg, sizeof(struct msgbuf) - sizeof(long), 0, 0);
    printf("%s: %d\n", msg->mtext, msg->id);
}

void parseLine(char *buf, struct msgbuf *msg) {
    strcpy(msg->mtext, buf);
    char *type = strtok(buf, " \n");
    if (strcmp(type, "MIRROR") == 0) {
        msg->mtype = MIRROR;
        strcpy(msg->mtext, strtok(NULL, " \n"));
    }
    else if (strcmp(type, "CALC") == 0) {
        msg->mtype = CALC;
    }
    else if (strcmp(type, "TIME") == 0) {
        msg->mtype = TIME;
        strcpy(msg->mtext, "");
    }
    else if (strcmp(type, "END") == 0) {
        msg->mtype = END;
        strcpy(msg->mtext, "end");
    }
    else {
        msg->mtype = UNKNOWN;
        strcpy(msg->mtext, "");
    }
}

void commandsFromFile(FILE *f, struct msgbuf *msg, int serverq, int clientq) {
    char *buf = (char *)calloc(100, sizeof(char));
    while (fgets(buf, 100, f) != NULL) {
        printf("%s", buf);
        if (strlen(buf) > MAX_LENGTH) {
            printf("Command too long\n");
            continue;
        }
        parseLine(buf, msg);
        if(msgsnd(serverq, msg, sizeof(struct msgbuf) - sizeof(long), 0) == -1) {
            printf("Sending failed\n");
        }
        if(strcmp(msg->mtext, "end") == 0) return;
        msgrcv(clientq, msg, sizeof(struct msgbuf) - sizeof(long), 0, 0);
        printf("%s\n", msg->mtext);
    }
}

void commandsFromStdin(FILE *f, struct msgbuf *msg, int serverq, int clientq) {
    char *buf = (char *)calloc(100, sizeof(char));
    int timeToEnd = 0;
    while (fgets(buf, 100, f) != NULL) {
        if (strlen(buf) > MAX_LENGTH) {
            printf("Command too long\n");
            continue;
        }
        if (strcmp(buf, "END\n") == 0) timeToEnd = 1;
        parseLine(buf, msg);
        if(msgsnd(serverq, msg, sizeof(struct msgbuf) - sizeof(long), 0) == -1) {
            printf("Sending failed\n");
            raise(SIGINT);
        }
        if (timeToEnd) break;
        msgrcv(clientq, msg, sizeof(struct msgbuf) - sizeof(long), 0, 0);
        printf("%s\n", msg->mtext);
    }
}

int main (int argc, char *argv[]) {
    if (argc > 2) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    setSignals();

    int serverq, clientq;
    if ((serverq = msgget(ftok("test.txt", 3), 0)) == -1) {
        printf("Error with client->server queue\n");
        exit(1);
    }

    if ((clientq = msgget(IPC_PRIVATE, 0666)) == -1) {
        printf("Error with server->client queue\n");
        exit(1);
    }

    int id;
    struct msgbuf msg;

    init(&msg, serverq, clientq);
    id = msg.id;

    FILE *f;
    if(argc == 2) {
        if ((f = fopen(argv[1], "r")) == NULL) {
            printf("Something went wrong\n");
            exit(1);
        }
        commandsFromFile(f, &msg, serverq, clientq);
    }
    else {
        f = stdin;
        commandsFromStdin(f, &msg, serverq, clientq);
    }

    if(msgctl(clientq, IPC_RMID, NULL) == 0) {
        printf("Success\n");
    }

    return 0;
}