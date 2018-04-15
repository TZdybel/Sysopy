//
// Created by tzdybel on 15.04.18.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    int N = (int)strtol(argv[2], NULL, 10);

    printf("My PID: %d\n", getpid());

    char *pid = (char *)calloc(12, sizeof(char));
    char *buf = (char *)calloc(100, sizeof(char));
    sprintf(pid, "%d", getpid());
    srand(time(NULL));
    while (N > 0) {
        FILE *f = fopen(argv[1], "w");
        sleep((unsigned int)rand()%5 + 1);
        fwrite("PID: ", 1, 5, f);
        fwrite(pid, 1, strlen(pid), f);
        fwrite(", ", 1, 2, f);
        FILE *date = popen("date", "r");
        fgets(buf, 100, date);
        pclose(date);
        printf("%s", buf);
        fwrite(buf, 1, 100, f);
        N--;
        fclose(f);
    }

    free(pid);
    free(buf);
}