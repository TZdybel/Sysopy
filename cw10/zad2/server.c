#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "additions.h"

int inetfd, unixfd;
int epoll;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
client clients[MAX_CLIENTS];
int activity_of_clients[MAX_CLIENTS];
int counter = 1;
pthread_t pingthread, commandthread;
char *unixpath;

void closeall();

void init(char *portstr, char *path) {
    unixpath = (char *)calloc(108, sizeof(char));
    strcpy(unixpath, path);

    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clients[i].addrlen = 0;
        clients[i].src_addr = (struct sockaddr *)calloc(1, sizeof(struct sockaddr));
    }
    for (int i = 0; i < MAX_CLIENTS; ++i) activity_of_clients[i] = -1;

    uint16_t port = (uint16_t) strtol(portstr, NULL, 10);
    if (port < 1024 || port > 65535) {
        printf("Error with number of port\n");
        exit(1);
    }
    if (strlen(path) > 108 || strlen(path) < 1) {
        printf("Error with path name\n");
        exit(1);
    }

    if ((inetfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        printf("Error with creating INET socket\n");
        exit(1);
    }
    if ((unixfd = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1) {
        printf("Error with creating UNIX socket\n");
        exit(1);
    }

    struct sockaddr_in inetaddr;
    inetaddr.sin_family = AF_INET;
    inetaddr.sin_port = htons(port);
    inetaddr.sin_addr.s_addr = INADDR_ANY;

    struct sockaddr_un unixaddr;
    unixaddr.sun_family = AF_UNIX;
    strcpy(unixaddr.sun_path, unixpath);

    if (bind(inetfd, (const struct sockaddr *)&inetaddr, sizeof(inetaddr)) == -1 ||
        bind(unixfd, (const struct sockaddr *)&unixaddr, sizeof(unixaddr)) == -1) {
        printf("Error with binding sockets\n");
        exit(1);
    }

    if ((epoll = epoll_create1(0)) == -1) {
        printf("Error with creating epoll\n");
        closeall();
        exit(1);
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLPRI;

    event.data.fd = inetfd;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, inetfd, &event) == -1) {
        printf("Error with adding inet socket to epoll\n");
        closeall();
        exit(1);
    }

    event.data.fd = unixfd;
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, unixfd, &event) == -1) {
        printf("Error with adding inet socket to epoll\n");
        closeall();
        exit(1);
    }
}

void closeall() {
    close(inetfd);
    close(unixfd);
    unlink(unixpath);

    free(unixpath);
    pthread_cancel(pingthread);
    pthread_cancel(commandthread);
}

void register_client(int socketfd) {
    struct sockaddr *src_addr = (struct sockaddr *)calloc(1, sizeof(struct sockaddr));
    socklen_t addrlen = sizeof(struct sockaddr);
    uint16_t length;
    if (recvfrom(socketfd, &length, 2, 0, src_addr, &addrlen) == -1) {
        printf("Error with receiving\n");
        exit(1);
    }

    char *clientname = (char *)calloc(length + 1, sizeof(char));
    if (recvfrom(socketfd, clientname, length, 0, src_addr, &addrlen) == -1) {
        printf("Error with receiving\n");
        exit(1);
    }

    pthread_mutex_lock(&clients_mutex);
    int index = -1;
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clients[i].addrlen == 0) {
            index = i;
            break;
        }
        if (strcmp(clients[i].name, clientname) == 0) {
            uint8_t type = NAME_IN_USE;
            sendto(socketfd, &type, 1, 0, src_addr, addrlen);
            pthread_mutex_unlock(&clients_mutex);
            return;
        }
    }
    if (index == -1) {
        uint8_t type = TOO_MANY_CLIENTS;
        sendto(socketfd, &type, 1, 0, src_addr, addrlen);
        pthread_mutex_unlock(&clients_mutex);
        return;
    }

    client client;
    client.src_addr = src_addr;
    client.addrlen = addrlen;
    strcpy(client.name, clientname);
    clients[index] = client;
    activity_of_clients[index] = 1;
    pthread_mutex_unlock(&clients_mutex);

    uint8_t type = REGISTERED;
    sendto(socketfd, &type, 1, 0, src_addr, addrlen);
    printf("Client %s registered\n", clientname);
    free(clientname);
}

void delete_client(int socketfd) {
    struct sockaddr *src_addr = (struct sockaddr *)calloc(1, sizeof(struct sockaddr));
    socklen_t addrlen = sizeof(struct sockaddr);
    uint16_t length;
    recvfrom(socketfd, &length, 2, 0, src_addr, &addrlen);
    char *name = (char *)calloc(length + 1, sizeof(char));
    recvfrom(socketfd, name, length, 0, src_addr, &addrlen);

    int deleted = 0;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (strcmp(clients[i].name, name) == 0) {
            deleted = 1;
            clients[i].addrlen = 0;
            strcpy(clients[i].name, "");
            activity_of_clients[i] = -1;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    if (deleted) printf("Client %s deleted from database\n", name);
    free(name);
}

void get_result(int socketfd) {
    struct sockaddr *src_addr = (struct sockaddr *)calloc(1, sizeof(struct sockaddr));
    socklen_t addrlen = sizeof(struct sockaddr);
    uint16_t length;
    recvfrom(socketfd, &length, 2, 0, src_addr, &addrlen);
    result result;
    recvfrom(socketfd, &result, length, 0, src_addr, &addrlen);
    int index;
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if(strcmp(clients[i].name, result.clientname) == 0) {
            index = i;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    printf("Got a result of order no. %d from client %s - result: %d\n", result.ID, clients[index].name, result.result);
}

void receive_message(int socketfd) {
    struct sockaddr *src_addr = (struct sockaddr *)calloc(1, sizeof(struct sockaddr));
    socklen_t addrlen = sizeof(struct sockaddr);
    uint8_t type;
    uint16_t length;
    char *name;
    if (recvfrom(socketfd, &type, 1, 0, src_addr, &addrlen) == -1) {
        printf("Error with receiving\n");
        exit(1);
    }

    switch(type) {
        case REGISTER:
            register_client(socketfd);
            break;
        case ANSWER_TO_PING:
            recvfrom(socketfd, &length, 2, 0, src_addr, &addrlen);
            name = (char *)calloc(length + 1, sizeof(char));
            recvfrom(socketfd, name, length, 0, src_addr, &addrlen);
            pthread_mutex_lock(&clients_mutex);
            for (int i = 0; i < MAX_CLIENTS; ++i) {
                if(strcmp(clients[i].name, name) == 0) {
                    activity_of_clients[i] = 1;
//                    printf("Client %s answered to ping\n", clients[i].name);
                    break;
                }
            }
            pthread_mutex_unlock(&clients_mutex);
            break;
        case DELETE:
            delete_client(socketfd);
            break;
        case RESULT:
            get_result(socketfd);
            break;
        default:
            break;
    }
}

void *ping_clients(void *args) {
    while (1) {
        sleep(5);
//        printf("Pinging\n");
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (activity_of_clients[i] == 1) {
                uint8_t type = PING;
                if (clients[i].src_addr->sa_family == AF_INET) sendto(inetfd, &type, 1, 0, clients[i].src_addr, clients[i].addrlen);
                else sendto(unixfd, &type, 1, 0, clients[i].src_addr, clients[i].addrlen);
                activity_of_clients[i] = 0;
            } else if (activity_of_clients[i] == 0) {
                printf("Deleting client %s because of inactivity\n", clients[i].name);
                activity_of_clients[i] = -1;
                clients[i].addrlen = 0;
                strcpy(clients[i].src_addr->sa_data, "");
                clients[i].src_addr->sa_family = 0;
                strcpy(clients[i].name, "");
            }
        }
        pthread_mutex_unlock(&clients_mutex);
        sleep(5);
    }
}

void *send_commands(void *args) {
    srand((unsigned int)time(NULL));
    operation operation;
    while(1) {
        operation.ID = counter;
        counter++;
        struct timespec time;
        time.tv_nsec = 100000000;
        time.tv_sec = 0;
        nanosleep(&time, NULL);         //żeby ładniej się wyświetlało
        printf("Enter some operation: ");
        char *command = (char *)calloc(200, sizeof(char));
        fgets(command, 200, stdin);
        if (strchr(command, '+') == NULL && strchr(command, '-') == NULL && strchr(command, '/') == NULL && strchr(command, '*') == NULL) {
            printf("Wrong command\n");
            continue;
        }
        int tmp = (int) strcspn(command, "+-/*");
        char op = command[tmp];
        int arg1 = (int) strtol(strtok(command, " +-/*"), NULL, 10);
        int arg2 = (int) strtol(strtok(NULL, " +-/*"), NULL, 10);
        operation.arg1 = arg1;
        operation.arg2 = arg2;
        operation.operator = op;

        uint8_t type = ORDER;
        uint16_t length = (uint16_t) sizeof(operation);

        client client;
        client.addrlen = 0;
//        printf("Searching for a client...\n");
        while (client.addrlen == 0) {
            client = clients[rand()%MAX_CLIENTS];
            if (client.addrlen != 0) {
                pthread_mutex_lock(&clients_mutex);
            }
        }
//        printf("Client found!\n");
//        printf("Send to client %s\n", client.name);
        int socketfd;
        if (client.src_addr->sa_family == AF_UNIX) socketfd = unixfd;
        else socketfd = inetfd;
        sendto(socketfd, &type, 1, 0, client.src_addr, client.addrlen);
        sendto(socketfd, &length, 2, 0, client.src_addr, client.addrlen);
        sendto(socketfd, &operation, length, 0, client.src_addr, client.addrlen);
        pthread_mutex_unlock(&clients_mutex);
    }
}

void intReact(int signum) {
    printf("\nStopping server...\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    init(argv[1], argv[2]);
    pthread_create(&pingthread, NULL, ping_clients, NULL);
    pthread_create(&commandthread, NULL, send_commands, NULL);
    atexit(closeall);
    signal(SIGINT, intReact);

    while (1) {
        struct epoll_event event;
        if (epoll_wait(epoll, &event, 1, -1) == -1){
            printf("Did not receive any events from epoll\n");
            exit(1);
        }
        if (event.data.fd == unixfd) receive_message(unixfd);
        else if (event.data.fd == inetfd) receive_message(inetfd);
    }
}
