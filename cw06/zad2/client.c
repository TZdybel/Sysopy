//
// Created by tzdybel on 21.04.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <memory.h>
#include <signal.h>
#include <mqueue.h>
#include <errno.h>
#include "posix.h"

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

void init(char *msg, mqd_t serv, mqd_t clientq, char *name) {
    sprintf(msg, "%d%s", INIT, name);

    if(mq_send(serv, msg, 8192, 1) == -1) {
        printf("Sending failed\n");
        exit(1);
    }

    if(mq_receive(clientq, msg, 8192, NULL) == -1) {
        printf("Something went wrong\n");
    }
    printf("%s\n", &msg[1]);
}

void parseLine(char *buf, char *msg, char *id) {
    char *type = strtok(buf, " \n");
    if (strcmp(type, "MIRROR") == 0) {
        char *word = strtok(NULL, " \n");
        sprintf(msg, "%d%d%s %s", MIRROR, (int)strlen(word), word, id);
    }
    else if (strcmp(type, "CALC") == 0) {
        sprintf(msg, "%d%s %s", CALC, strtok(NULL, " "), id);
    }
    else if (strcmp(type, "TIME") == 0) {
        sprintf(msg, "%d %s", TIME, id);
    }
    else if (strcmp(type, "END") == 0) {
        sprintf(msg, "%d%s %s", END, "end", id);
    }
    else {
        sprintf(msg, "%d %s", UNKNOWN, id);
    }
}

void commandsFromFile(FILE *f, char *msg, mqd_t serv, mqd_t clientq, char *id) {
    char *buf = (char *)calloc(100, sizeof(char));
    while (fgets(buf, 100, f) != NULL) {
        printf("%s", buf);
        if (strlen(buf) > MAX_LENGTH) {
            printf("Command too long\n");
            continue;
        }
        parseLine(buf, msg, id);
        if(mq_send(serv, msg, 8192, 1) == -1) {
            printf("Sending failed\n");
        }
        if(strncmp(&msg[1], "end", 3) == 0) return;
        mq_receive(clientq, msg, 8192, NULL);
        printf("%s\n", &msg[1]);
    }
}

void commandsFromStdin(FILE *f, char *msg, mqd_t serv, mqd_t clientq, char *id) {
    char *buf = (char *)calloc(100, sizeof(char));
    int timeToEnd = 0;
    while (fgets(buf, 100, f) != NULL) {
        if (strlen(buf) > MAX_LENGTH) {
            printf("Command too long\n");
            continue;
        }
        if (strcmp(buf, "CLOSE\n") == 0) return;
        if (strcmp(buf, "END\n") == 0) timeToEnd = 1;
        parseLine(buf, msg, id);
        if(mq_send(serv, msg, 8192, 1) == -1) {
            printf("Sending failed\n");
            raise(SIGINT);
        }
        if (timeToEnd) break;
        mq_receive(clientq, msg, 8192, NULL);
        printf("%s\n", &msg[1]);
    }
}

int main (int argc, char *argv[]) {
    if (argc > 2) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    setSignals();

    mqd_t serv, clientq;
    if ((serv = mq_open("/server", O_WRONLY)) == -1) {
        printf("Error with client->server queue\n");
        exit(1);
    }

    char *pid = (char *)calloc(10, sizeof(char));
    sprintf(pid, "/%d", getpid());
    if ((clientq = mq_open(pid, O_CREAT | O_RDONLY, 0600, NULL)) == -1) {
        printf("Error with server->client queue\n");
        exit(1);
    }

    int idNumber;
    char *msg = (char *)calloc(8192, sizeof(char));

    init(msg, serv, clientq, pid);
    idNumber = (int)strtol(&msg[strlen(msg) - 2], NULL, 10);
    char *id = (char *)calloc(2, sizeof(char));
    strcpy(id, &msg[strlen(msg) - 2]);

    FILE *f;
    if(argc == 2) {
        if ((f = fopen(argv[1], "r")) == NULL) {
            printf("Something went wrong\n");
            exit(1);
        }
        commandsFromFile(f, msg, serv, clientq, id);
    }
    else {
        f = stdin;
        commandsFromStdin(f, msg, serv, clientq, id);
    }
    fclose(f);

    sprintf(msg, "%d%s", CLOSE, id);
    mq_send(serv, msg, 8192, 1);

    mq_close(clientq);
    if(mq_unlink(pid) == -1) printf("Cannot destroy queue\n");


    free(pid);
    free(msg);
    free(id);
    return 0;
}
