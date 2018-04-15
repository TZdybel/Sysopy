//
// Created by tzdybel on 24.03.18.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>

void freeTab(char **t, int size) {
    free(t[size-1]);
    free(t);
}

int charFrequency(const char* string, char c) {
    int count = 0;
    int i;
    for (i = 0; i < strlen(string); i++) {
        if (string[i] == c) {
            count++;
        }
    }
    return count;
}

int separateArguments(char *buf, char **words, int whichProcess) {
    char *pom = strtok(buf, " ");
    while (whichProcess > 0) {                      //toporne przesuwanie na odpowiednie słowo za każdym razem
        if (strcmp(pom, "|") == 0) whichProcess--;  //nie miałem innnego pomysłu
        pom = strtok(NULL, " ");
    }
    int size = 0;
    while (pom != NULL && strcmp(pom, "|") != 0) {
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

void commandWithPipe(char *buf) {
    int numOfProcesses = charFrequency(buf, '|');
    pid_t child = fork();
    if (child == 0) {
        while (numOfProcesses > 0) {
            int *fd = (int *) calloc(2, sizeof(int));
            pipe(fd);
            pid_t pid = fork();
            numOfProcesses--;
            if (pid == 0 && numOfProcesses != 0) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                continue;
            }
            if (pid == 0) {
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                char **words = (char **) calloc(10, sizeof(char *));
                int size = separateArguments(buf, words, 0);
                execvp(words[0], words);
                exit(1);
            } else {
                close(fd[1]);
                dup2(fd[0], STDIN_FILENO);
                int status;
                wait(&status);
                if (status != 0) {
                    exit(1);
                }
                char **words = (char **) calloc(10, sizeof(char *));
                int size = separateArguments(buf, words, numOfProcesses + 1);
                execvp(words[0], words);
                exit(1);
            }
        }
    } else {
        int status;
        wait(&status);
        if (status != 0) {
            printf("Something went wrong. Check input file for errors\n");
            exit(1);
        }
    }
}

void simpleCommand(char *buf) {
    char **words = (char **) calloc(10, sizeof(char *));
    int size = separateArguments(buf, words, 0);
    pid_t child_pid = fork();
    if (child_pid == 0) {
        execvp(words[0], words);
        exit(1);
    } else {
        int status;
        wait(&status);
        freeTab(words, size);
        if (status != 0) {
            printf("Something went wrong. Check input file for errors\n");
            exit(1);
        }
    }
}

int main (int argc, char *argv[]) {
    if (argc != 2) {
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
        if (strchr(buf, (int)'|') != NULL) commandWithPipe(buf);
        else simpleCommand(buf);
    }

    fclose(file);
    return 0;
}
