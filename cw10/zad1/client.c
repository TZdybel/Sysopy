#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/un.h>
#include <signal.h>
#include "additions.h"

int socketfd;
char *name;

void web_init(char *serveraddr) {
    strtok(serveraddr, ":");
    char *portstr = strtok(NULL, ":");
    uint16_t port = (uint16_t) strtol(portstr, NULL, 10);
    if (port < 1024 || port > 65535) {
        printf("Error with number of port\n");
        exit(1);
    }

    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Error with creating INET socket\n");
        exit(1);
    }

    struct sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);

    struct in_addr inp;
    if (inet_aton(serveraddr, &inp) == -1) {
        perror("Error with IP address");
    }

    sockaddr.sin_addr = inp;

    if(connect(socketfd, (const struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1) {
        perror("Error with connecting to server");
    }
}

void unix_init(char *path) {
    if (strlen(path) > 108 || strlen(path) < 1) {
        printf("Error with path name\n");
        exit(1);
    }

    if ((socketfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        printf("Error with creating UNIX socket\n");
        exit(1);
    }

    struct sockaddr_un sockaddr;
    sockaddr.sun_family = AF_UNIX;
    strcpy(sockaddr.sun_path, path);

    if (connect(socketfd, (const struct sockaddr *) &sockaddr, sizeof(sockaddr)) == -1) {
        perror("Error with connecting to server");
    }
}

void register_on_server(char *name) {
    uint8_t type = REGISTER;
    uint16_t length = (uint16_t) strlen(name);
    if (send(socketfd, &type, 1, 0) == -1 || send(socketfd, &length, 2, 0) == -1 || send(socketfd, name, length, 0) == -1) {
        printf("Error with sending register messages\n");
        exit(1);
    }
    recv(socketfd, &type, 1, 0);

    switch(type) {
        case REGISTERED:
            printf("Successfully registered\n");
            break;
        case TOO_MANY_CLIENTS:
            printf("Server full\n");
            exit(0);
        case NAME_IN_USE:
            printf("Name in use\n");
            exit(0);
        default:
            break;
    }
}

void closeall() {
    shutdown(socketfd, SHUT_RDWR);
    close(socketfd);
}

void intReact(int signum) {
    printf("\nReceived signal no. %d - ending client process\n", signum);
    uint8_t type = DELETE;
    send(socketfd, &type, 1, 0);
    uint16_t length = (uint16_t) strlen(name);
    send(socketfd, &length, 2, 0);
    send(socketfd, name, length, 0);
    exit(0);
}

void setsignals() {
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);
    signal(SIGINT, intReact);
}

void answer_to_order() {
    printf("ORDER\n");
    uint16_t length;
    recv(socketfd, &length, 2, 0);
    operation operation;
    recv(socketfd, &operation, length, 0);

    result result;
    result.result = 0;
    result.ID = operation.ID;

    switch(operation.operator) {
        case '+':
            result.result = operation.arg1 + operation.arg2;
            break;
        case '-':
            result.result = operation.arg1 - operation.arg2;
            break;
        case '*':
            result.result = operation.arg1 * operation.arg2;
            break;
        case '/':
            result.result = operation.arg1 / operation.arg2;
            break;
        default:
            break;
    }

    uint8_t type = RESULT;
    send(socketfd, &type, 1, 0);
    length = sizeof(result);
    send(socketfd, &length, 2, 0);
    send(socketfd, &result, length, 0);
}

int main (int argc, char *argv[]) {
    if (argc != 4) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    name = argv[1];
    char *serveraddr = argv[3];
    if (strcmp(argv[2], "web") == 0) web_init(serveraddr);
    else if (strcmp(argv[2], "unix") == 0) unix_init(serveraddr);
    else {
        printf("Wrong connection type\n");
        exit(1);
    }

    setsignals();
    atexit(closeall);

    register_on_server(name);

    uint8_t type;
    while(1) {
        recv(socketfd, &type, 1, 0);
        if (type == ORDER) {
            answer_to_order();
        }
        else if (type == PING){
            printf("Received PING\n");
            type = ANSWER_TO_PING;
            send(socketfd, &type, 1, 0);
            uint16_t length = (uint16_t) strlen(name);
            send(socketfd, &length, 2, 0);
            send(socketfd, name, length, 0);
        }
    }
}