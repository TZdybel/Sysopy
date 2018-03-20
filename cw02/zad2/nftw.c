//
// Created by tzdybel on 20.03.18.
//
#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ftw.h>

int mode;
time_t date;

char* formatdate(char* str, time_t val)
{
    strftime(str, 20, "%d.%m.%Y %H:%M:%S", localtime(&val));
    return str;
}

int printProperties(const char *path, const struct stat *buf, int typeflag, struct FTW *ftwbuf) {
    if(typeflag == FTW_F) {
        int satisfyCondition = 0;

        if (((mode == 0 && buf->st_mtime < date)) || ((mode == 1 && buf->st_mtime > date + (24 * 60 * 60))) || ((mode == 2 && buf->st_mtime >= date && buf->st_mtime <= date + (24 * 60 * 60)))) satisfyCondition = 1;
        if (satisfyCondition == 1) {
            char datestr[20];
            printf("Bezwzgledna sciezka: %s  ", path);
            printf("Size: %d  ", (int) buf->st_size);
            printf("Modify: %s  ", formatdate(datestr, buf->st_mtime));

            int stMode = buf->st_mode;
            printf(stMode & S_IRUSR ? "r" : "-");
            printf(stMode & S_IWUSR ? "w" : "-");
            printf(stMode & S_IXUSR ? "x" : "-");
            printf(stMode & S_IRGRP ? "r" : "-");
            printf(stMode & S_IWGRP ? "w" : "-");
            printf(stMode & S_IXGRP ? "x" : "-");
            printf(stMode & S_IROTH ? "r" : "-");
            printf(stMode & S_IWOTH ? "w" : "-");
            printf(stMode & S_IXOTH ? "x" : "-");
            printf("\n");
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    char *sign = argv[2];
    if (strlen(sign) == 1) {
        if (strcmp(sign, "<") == 0) mode = 0;
        else if (strcmp(sign, ">") == 0) mode = 1;
        else if (strcmp(sign, "=") == 0) mode = 2;
        else {
            printf("Invalid sign\n");
            exit(1);
        }
    }
    else {
        printf("Invalid sign\n");
        exit(1);
    }

    char *data = argv[3];
    char day[3], month[3], year[5];
    struct tm tm1;

    memcpy(day, &data[0], 2);
    memcpy(month, &data[3], 2);
    memcpy(year, &data[6], 4);
    day[2] = '\0';
    month[2] = '\0';
    year[4] = '\0';

    tm1.tm_year = (int) (strtol(year, NULL, 10) - 1900);
    tm1.tm_mday = (int) strtol(day, NULL, 10);
    tm1.tm_mon = (int) (strtol(month, NULL, 10) - 1);
    tm1.tm_hour = tm1.tm_min = tm1.tm_sec = 0;
    tm1.tm_isdst = 0;
    date = mktime(&tm1);

    nftw(argv[1], printProperties, 100, FTW_PHYS);

    return 0;

}