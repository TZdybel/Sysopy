//
// Created by tzdybel on 15.04.18.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <wait.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    mkfifo(argv[1], 0666);
    FILE *f = fopen(argv[1], "r");
    if (f == NULL) {
        printf("Something went wrong with pipe\n");
        exit(1);
    }

    char *buf = (char *)calloc(50, sizeof(char));
    while (1) {
        fread(buf, 1, 1, f);                        //dlaczego tak działa a fread(buf, 1, 50, f) nie??????????
        printf("%s", buf);
    }

//    fclose(f);
}