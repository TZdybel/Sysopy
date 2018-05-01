#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <signal.h>
#include <zconf.h>
#include <mqueue.h>
#include <fcntl.h>
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

int numOfDigits(int x) {
    int i = 0;
    while (x > 0) {
        i++;
        x /= 10;
    }
    return i;
}

void mirror(char *msg) {
    int len = (int)strtol(&msg[1], NULL, 10);
    char *tmp = (char *)calloc((size_t)len, sizeof(char));
    int digits = numOfDigits(len);
    for (int i = 1 + digits; i <= len + digits; ++i)
    {
        tmp[len + digits - i] = msg[i];
    }
    sprintf(msg, "%d%s", REPLY, tmp);
}

void calc(char *msg) {
    int x = (int)strtol(&msg[1], NULL, 10);
    int y = (int)strtol(&msg[1 + numOfDigits(x) + 1], NULL, 10);
    char sign = msg[1+numOfDigits(x)];
    int result = 0;
    switch (sign) {
        case '+':
            result = x+y;
            sprintf(msg, "%d%d", REPLY, result);
            break;
        case '-':
            result = x-y;
            sprintf(msg, "%d%d", REPLY, result);
            break;
        case '/':
            result = x/y;
            sprintf(msg, "%d%d", REPLY, result);;
            break;
        case '*':
            result = x*y;
            sprintf(msg, "%d%d", REPLY, result);
            break;
        default:
            sprintf(msg, "%d%s", REPLY, "Inappropriate sign");
            break;
    }
}

void askTime(char *msg) {
    time_t secs = time(NULL);
    char *t = ctime(&secs);
    sprintf(msg, "%d%s", REPLY, t);
}

void initialize_client(mqd_t *clients, int nextID, char *msg) {
    if ((clients[nextID] = mq_open(&msg[1], O_WRONLY)) == -1) {
        printf("Error with server->client queue\n");
        return;
    }
    sprintf(msg, "%d%s: %d", REPLY, "Assigning ID", nextID);
}

int main() {
    setSignals();

    char *msg = (char *)calloc(8192, sizeof(char));

    mqd_t serv;
    if ((serv = mq_open("/server", O_CREAT | O_RDONLY, 0600, NULL)) < 0) {
        printf("Error with client->server queue\n");
        exit(1);
    }

    mqd_t *clients = (mqd_t *)calloc(MAX_CLIENTS, sizeof(mqd_t));
    int nextID = 0;

    struct mq_attr attr;
    int continueWork = 1, timeToEnd = 0;
    while(continueWork > 0) {
        mq_receive(serv, msg, 8192, NULL);
        int clientID = (int)strtol(&msg[strlen(msg) - 2], NULL, 10);
        int task = (int)msg[0] - 48;
        switch(task) {
            case INIT:
                initialize_client(clients, nextID, msg);
                clientID = nextID;
                nextID++;
                break;
            case MIRROR:
                mirror(msg);
                break;
            case CALC:
                calc(msg);
                break;
            case TIME:
                askTime(msg);
                break;
            case CLOSE:
                if (mq_close(clients[clientID]) == -1) {
                    printf("Cannot close server->client queue\n");
                }
                if(timeToEnd) continueWork--;
                continue;
            case END:
                if(timeToEnd) {
                    continueWork--;
                    continue;
                }
                mq_getattr(serv, &attr);
                continueWork = (int)attr.mq_curmsgs;
                timeToEnd = 1;
                continue;
            default:
                sprintf(msg, "%d%s", REPLY, "Unknown command");
                break;
        }
        if (mq_send(clients[clientID], msg, 8192, 1) == -1) {
            printf("Sending failed\n");
        }
        if (timeToEnd) continueWork--;
    }
    printf("Closing server...\n");
    for (int i = 0; i < nextID; ++i) {
        mq_close(clients[i]);
    }
    if(mq_unlink("/server") == -1) printf("Cannot destroy client->server queue");

    free(clients);
    free(msg);
    return 0;
}
