//
// Created by tzdybel on 24.03.18.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>
#include <sys/time.h>
#include <sys/resource.h>

int separateArguments(char *buf, char **words) {
    char *pom = strtok(buf, " ");
    int size = 0;
    while (pom != NULL) {
        if (strchr(pom, (int)'\n') == NULL) {
            words[size] = pom;
            size++;
        }
        else {
            size_t len = strlen(pom);
            words[size] = (char *)calloc(len, sizeof(char));
            memcpy(words[size], pom, len - 1);
            size++;
        }
        pom = strtok(NULL, " ");
    }
    return size;
}

void setResourcesLimits(int seconds, int megabytes) {
    struct rlimit setCPU, setAS;
    setCPU.rlim_cur = (rlim_t)seconds*2/3;
    setCPU.rlim_max = (rlim_t)seconds;
    setrlimit(RLIMIT_CPU, &setCPU);
    int bytes = megabytes*1024*1024;
    setAS.rlim_cur = (rlim_t)bytes*2/3;
    setAS.rlim_max = (rlim_t)bytes;
    setrlimit(RLIMIT_AS, &setAS);
}

int main (int argc, char *argv[]) {
    if (argc != 4) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        printf("Wrong file path\n");
        exit(1);
    }

    char *buf = (char *)calloc(1000, sizeof(char));
    while (fgets(buf, 999, file) != NULL) {
        printf("%s", buf);
        char **words = (char **) calloc(10, sizeof(char *));
        int size = separateArguments(buf, words);
        pid_t child_pid = fork();
        if (child_pid == 0) {
            setResourcesLimits((int)strtol(argv[2], NULL, 10), (int)strtol(argv[3], NULL, 10));
//            struct rlimit getCPU, getAS;
//            getrlimit(RLIMIT_CPU, &getCPU);
//            getrlimit(RLIMIT_AS, &getAS);
//            printf("CPU - soft: %lld, hard: %lld\n", (long long)getCPU.rlim_cur, (long long)getCPU.rlim_max);
//            printf("AS - soft: %lld, hard: %lld\n", (long long)getAS.rlim_cur, (long long)getAS.rlim_max);
            execvp(words[0], words);
            exit(1);
        }
        else {
            int status;
            wait(&status);
            if(status != 0) {
                printf("Something went wrong. Check input file for errors\n");
                break;
            }
            struct rusage usage;
            getrusage(RUSAGE_CHILDREN, &usage);
            printf("User CPU time used: %d.%d seconds,  system CPU time used: %d.%d seconds\n", (int)usage.ru_utime.tv_sec, (int)usage.ru_utime.tv_usec, (int)usage.ru_stime.tv_sec, (int)usage.ru_stime.tv_usec);
        }
        free(words[size]);
        free(words);
    }

    fclose(file);
    return 0;
}
