#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <zconf.h>
#include <signal.h>

FILE *text;
char **buf;
int P, K, N, L, searchType, dataPrintType, nk, producer_idx = 0, consumer_idx = 0, timeToEnd = 0;
pthread_mutex_t *buf_mutexes;
pthread_mutex_t idx_mutex;
pthread_mutex_t consumer_idx_mutex;
pthread_mutex_t text_mutex;

void initializeMutexes() {
    buf_mutexes = (pthread_mutex_t *)calloc(N, sizeof(pthread_mutex_t));
    for (int i = 0; i < N; ++i) {
        pthread_mutex_init(&buf_mutexes[i], NULL);
    }
    pthread_mutex_init(&idx_mutex, NULL);
    pthread_mutex_init(&text_mutex, NULL);
}

char *getParameters(char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        printf("Wrong filepath, check input data\n");
        exit(1);
    }
    char *tmp = (char *)calloc(150, sizeof(char));
    if (fgets(tmp, 150, f) == NULL) {
        printf("Something is wrong with input file\n");
        exit(1);
    }
    char *parameters;
    char *filename = (char *)calloc(100, sizeof(char));
    parameters = strtok(tmp, " \n");
    P = (int)strtol(parameters, NULL, 10);
    parameters = strtok(NULL, " \n");
    K = (int)strtol(parameters, NULL, 10);
    parameters = strtok(NULL, " \n");
    N = (int)strtol(parameters, NULL, 10);
    parameters = strtok(NULL, " \n");
    if (strlen(parameters) > 50) {
        printf("File name too long\n");
        fclose(f);
        free(tmp);
        free(filename);
        exit(1);
    }
    strcpy(filename, parameters);
    parameters = strtok(NULL, " \n");
    L = (int)strtol(parameters, NULL, 10);
    parameters = strtok(NULL, " \n");
    if (strcmp(parameters, "<") == 0) searchType = 1;
    else if (strcmp(parameters, "=") == 0) searchType = 2;
    else if (strcmp(parameters, ">") == 0) searchType = 3;
    else {
        printf("Wrong sign of search type\n");
        fclose(f);
        free(tmp);
        free(filename);
        exit(1);
    }
    parameters = strtok(NULL, " \n");
    dataPrintType = (int)strtol(parameters, NULL, 10);
    if (dataPrintType != 1 && dataPrintType != 0) {
        printf("Wrong data print type\n");
        fclose(f);
        free(tmp);
        free(filename);
        exit(1);
    }
    parameters = strtok(NULL, " \n");
    nk = (int)strtol(parameters, NULL, 10);
    if (nk < 0) {
        printf("nk must be 0 or higher\n");
        fclose(f);
        free(tmp);
        free(filename);
        exit(1);
    }
    fclose(f);
    free(tmp);
    return filename;
}

void *producer(void *args) {
    char *filename = (char *)args;
    char *line = (char *)calloc(1000, sizeof(char));
    while(1) {
        if (dataPrintType) printf("Producer %lld - another loop\n", (long long)pthread_self());
        pthread_mutex_lock(&idx_mutex);
        if (dataPrintType) printf("Producer %lld - setting local index\n", (long long)pthread_self());
        int localIndex = producer_idx;
        producer_idx++;
        if (producer_idx == N) producer_idx = 0;
        pthread_mutex_unlock(&idx_mutex);
        pthread_mutex_lock(&buf_mutexes[localIndex]);
        if (buf[localIndex] != NULL) {
            pthread_mutex_unlock(&buf_mutexes[localIndex]);
            if (dataPrintType) printf("Producer %lld - waiting for empty buffer\n", (long long)pthread_self());
            while (buf[localIndex] != NULL) {
                if (timeToEnd == 1) break;
            }
            if (dataPrintType) printf("Producer %lld - no more waiting\n", (long long)pthread_self());
            pthread_mutex_lock(&buf_mutexes[localIndex]);
        }
        pthread_mutex_lock(&text_mutex);
        if (fgets(line, 1000, text) != NULL) {
            if (dataPrintType) printf("Producer %lld - writing line to buffer\n", (long long)pthread_self());
            pthread_mutex_unlock(&text_mutex);
            buf[localIndex] = (char *)calloc(strlen(line)+1, sizeof(char));
            strcpy(buf[localIndex], line);
            pthread_mutex_unlock(&buf_mutexes[localIndex]);
        } else {
            if(nk == 0) {
                free(line);
                pthread_mutex_unlock(&text_mutex);
                pthread_mutex_unlock(&buf_mutexes[localIndex]);
                if (dataPrintType) printf("Producer %lld - End of file\n", (long long)pthread_self());
                timeToEnd = 1;
                pthread_exit(0);
            }
            if (dataPrintType) printf("Producer %lld - text reset\n", (long long)pthread_self());
            fclose(text);
            text = fopen(filename, "r");
            pthread_mutex_unlock(&text_mutex);
            pthread_mutex_unlock(&buf_mutexes[localIndex]);
        }
        if (timeToEnd == 1 && nk > 0) {
            printf("Times up, ending...\n");
            break;
        }
    }
    free(line);
}

int checkLength(char *text) {
    int x = (int)strcspn(text, "\t\n\v\f\r");
    switch(searchType) {
        case 1:
            if (x < L) return 0;
            else return 1;
        case 2:
            if (x == L) return 0;
            else return 1;
        case 3:
            if (x > L) return 0;
            else return 1;
        default:
            return 1;
    }
}

void *consumer(void *arg) {
    while(1) {
        char *line = (char *)calloc(1000, sizeof(char));
        if (dataPrintType) printf("Consumer %lld - another loop\n", (long long)pthread_self());
        pthread_mutex_lock(&consumer_idx_mutex);
        if (dataPrintType) printf("Consumer %lld - setting local index\n", (long long)pthread_self());
        int localIndex = consumer_idx;
        consumer_idx++;
        if (consumer_idx == N) consumer_idx = 0;
        pthread_mutex_unlock(&consumer_idx_mutex);
        pthread_mutex_lock(&buf_mutexes[localIndex]);
        if (buf[localIndex] == NULL) {
            pthread_mutex_unlock(&buf_mutexes[localIndex]);
            if (dataPrintType) printf("Consumer %lld - waiting for buffer to be filled\n", (long long)pthread_self());
            while (buf[localIndex] == NULL) {                  //Boże, wybacz mi
                if (timeToEnd == 1) {
                    free(line);
                    if (dataPrintType) printf("Consumer %lld - ending\n", (long long)pthread_self());
                    pthread_exit(0);
                }
            }
            if (dataPrintType) printf("Consumer %lld - no more waiting\n", (long long)pthread_self());
            pthread_mutex_lock(&buf_mutexes[localIndex]);
        }
        if (buf[localIndex] == NULL) {                         //to też mi wybacz
            pthread_mutex_unlock(&buf_mutexes[localIndex]);
            continue;
        }
        if (dataPrintType) printf("Consumer %lld - getting text from buffer\n", (long long)pthread_self());
        strcpy(line, buf[localIndex]);
        free(buf[localIndex]);
        buf[localIndex] = NULL;
        if (dataPrintType) printf("Consumer %lld - checking line\n", (long long)pthread_self());
        if (checkLength(line) == 0) printf("Index: %d, Length: %d - %s", localIndex, (int)strcspn(line, "\t\n\v\f\r"), line);
        pthread_mutex_unlock(&buf_mutexes[localIndex]);
        if (timeToEnd == 1 && nk > 0) {
            if (dataPrintType) printf("Consumer %lld - ending\n", (long long)pthread_self());
            break;
        }
        free(line);
    }
}

void freeBuf() {
    for (int i = 0; i < N; ++i) {
        if (buf[i] != NULL) free(buf[i]);
    }
    free(buf);
}

void intReact(int signum) {
    printf("Ending...");
    freeBuf();
    exit(0);
}

void setSignals() {
    sigset_t set;
    sigfillset(&set);
    if (nk == 0) sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);
    if (nk == 0) signal(SIGINT, intReact);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
	printf("Wrong number of arguments\n");
	exit(1);
    }
    setSignals();

    char *filepath = argv[1];
    char *filename = getParameters(filepath);
    initializeMutexes();
    buf = (char **)calloc(N, sizeof(char *));

    text = fopen(filename, "r");
    if (text == NULL) {
        printf("Wrong file name\n");
        free(filename);
        exit(1);
    }
    pthread_t *producers = (pthread_t *)calloc(P, sizeof(pthread_t));
    pthread_t *consumers = (pthread_t *)calloc(K, sizeof(pthread_t));
    for (int i = 0; i < P; ++i) {
        pthread_create(&producers[i], NULL, producer, (void *)filename);
    }
    for (int i = 0; i < K; ++i) {
        pthread_create(&consumers[i], NULL, consumer, NULL);
    }
    if (nk > 0) {
        sleep(nk);
        timeToEnd = 1;
    }
    for (int i = 0; i < P; ++i) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < K; ++i) {
        pthread_join(consumers[i], NULL);
    }

    freeBuf();
    fclose(text);
    return 0;
}
