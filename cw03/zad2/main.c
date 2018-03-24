//
// Created by tzdybel on 24.03.18.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>

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
        char **words = (char **) calloc(10, sizeof(char *));
        int size = separateArguments(buf, words);
        pid_t child_pid = fork();
        if (child_pid == 0) {
            execvp(words[0], words);
            exit(1);
        }
        else {
            int status;
            wait(&status);
            if(status != 0) {
                printf("Something went wrong. Check input file for errors\n");
                exit(1);
            }
        }
        free(words[size]);
        free(words);
    }

    fclose(file);
    return 0;
}
