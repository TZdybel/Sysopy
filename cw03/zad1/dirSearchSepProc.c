#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <wait.h>

char* formatdate(char* str, time_t val)
{
    strftime(str, 20, "%d.%m.%Y %H:%M:%S", localtime(&val));
    return str;
}

void printProperties(struct dirent *file, char *dirpath, int mode, time_t date) {
    struct stat *buf = malloc(sizeof(struct stat));
    char *tmp = malloc(sizeof(dirpath) + sizeof(file->d_name));
    strcpy(tmp, dirpath);
    strcat(tmp, file->d_name);
    lstat(tmp, buf);
    int satisfyCondition = 0;

    if (((mode == 0 && buf->st_mtime < date)) || ((mode == 1 && buf->st_mtime > date + (24*60*60))) || ((mode == 2 && buf->st_mtime >= date && buf->st_mtime <= date + (24*60*60)))) satisfyCondition = 1;
    if (satisfyCondition == 1) {
        char datestr[20];
        printf("Bezwzgledna sciezka: %s  ", tmp);
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

void searchDirTree(char *dirpath, int mode, time_t date) {
    DIR *dirp = opendir(dirpath);

    if (dirp == NULL) {
        printf("Wrong directory path\n");
        exit(1);
    }

    struct dirent *file;
    while ((file = readdir(dirp)) != NULL) {
        if (strcmp(file->d_name, ".") != 0 && strcmp(file->d_name, "..") != 0) {
            if (file->d_type == DT_REG) {
                printProperties(file, dirpath, mode, date);
            }
            else if (file->d_type == DT_DIR) {
                char *tmp = (char *)calloc(300, sizeof(char));
                strcpy(tmp, dirpath);
                strcat(tmp, file->d_name);
                strcat(tmp, "/");

                pid_t child_pid;
                child_pid = fork();
                if (child_pid == 0) {
                    searchDirTree(tmp, mode, date);
                    exit(0);
                } else {
                    wait(NULL);
                }
            }
        }
    }
    closedir(dirp);
}

int main (int argc, char *argv[]) {
    if (argc != 4) {
        printf("Wrong number of arguments\n");
        exit(1);
    }

    int mode;
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

    searchDirTree(argv[1], mode, mktime(&tm1));

    return 0;
}

