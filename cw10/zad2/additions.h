#define MAX_CLIENT_NAME 100
#define MAX_CLIENTS 2

#define REGISTER 1
#define TOO_MANY_CLIENTS 2
#define NAME_IN_USE 3
#define REGISTERED 4
#define ORDER 5
#define PING 6
#define ANSWER_TO_PING 7
#define DELETE 8
#define RESULT 9

typedef struct client {
    struct sockaddr *src_addr;
    socklen_t addrlen;
    char name[MAX_CLIENT_NAME];
} client;

typedef struct operation {
    int ID;
    char operator;
    int arg1;
    int arg2;
} operation;

typedef struct result {
    int ID;
    int result;
    char clientname[MAX_CLIENT_NAME];
} result;