#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>


void generateData(char text[], int block_size) {
    char set[53] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < block_size; ++i) {
        text[i] = set[rand()%52];
    }
}

void generate(char *filename, int numOfRecords, int sizeOfRecords) {
    char buf[sizeOfRecords];
    char newline = '\n';

    int file;
    file = open(filename, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);

    for (int i = 0; i < numOfRecords; ++i) {
        generateData(buf, sizeOfRecords);
        write(file, buf, sizeOfRecords);
        if (i != numOfRecords - 1) write(file, &newline, 1);
    }

    close(file);
}

void syssort(char *filename, int numOfRecords, int sizeOfRecords) {
    char buf1[sizeOfRecords], buf2[sizeOfRecords];
    unsigned char char1, char2;
    int file;

    file = open(filename, O_RDWR);

    for (int i = 1; i < numOfRecords; ++i) {
        lseek(file, i*(sizeOfRecords + 1), 0);
        read(file, buf1, sizeOfRecords);
        char1 = (unsigned char) buf1[0];
        lseek(file, 0, 0);

        for (int j = 0; j < i; ++j) {
            read(file, buf2, sizeOfRecords);
            char2 = (unsigned char) buf2[0];
            lseek(file, 1, 1);

            if (char1 < char2) {
                lseek(file, -(sizeOfRecords + 1), 1);
                write(file, buf1, sizeOfRecords);
                lseek(file, 1, 1);
                for (int k = j; k < i; ++k) {
                    read(file, buf1, sizeOfRecords);
                    lseek(file, -sizeOfRecords, 1);
                    write(file, buf2, sizeOfRecords);
                    for (int l = 0; l < sizeOfRecords; ++l) {
                        buf2[l] = buf1[l];
                    }
                    lseek(file, 1, 1);
                }
                break;
            }
        }
    }

    close(file);
}

void libsort(char *filename, int numOfRecords, int sizeOfRecords) {
    char buf1[sizeOfRecords], buf2[sizeOfRecords];
    unsigned char char1, char2;

    FILE *file = fopen(filename, "r+");

    if (file) {
        for (int i = 1; i < numOfRecords; ++i) {
            fseek(file, i * (sizeOfRecords + 1), 0);
            fread(buf1, 1, sizeOfRecords, file);
            char1 = (unsigned char) buf1[0];
            fseek(file, 0, 0);

            for (int j = 0; j < i; ++j) {
                fread(buf2, 1, sizeOfRecords, file);
                char2 = (unsigned char) buf2[0];
                fseek(file, 1, 1);

                if (char1 < char2) {
                    fseek(file, -(sizeOfRecords + 1), 1);
                    fwrite(buf1, 1, sizeOfRecords, file);
                    fseek(file, 1, 1);
                    for (int k = j; k < i; ++k) {
                        fread(buf1, 1, sizeOfRecords, file);
                        fseek(file, -sizeOfRecords, 1);
                        fwrite(buf2, 1, sizeOfRecords, file);
                        for (int l = 0; l < sizeOfRecords; ++l) {
                            buf2[l] = buf1[l];
                        }
                        fseek(file, 1, 1);
                    }
                    break;
                }
            }
        }
        fclose(file);
    }
}

void syscopy(char *filename1, char *filename2, int numOfRecords, int sizeOfRecords) {
    char buf[sizeOfRecords + 1];

    int file1, file2;
    file1 = open(filename1, O_RDONLY);
    file2 = open(filename2, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);

    for (int i = 0; i < numOfRecords; ++i) {
        read(file1, buf, sizeOfRecords + 1);
        if (i == numOfRecords - 1) write(file2, buf, sizeOfRecords);
        else write(file2, buf, sizeOfRecords + 1);
    }

    close(file1);
    close(file2);
}

void libcopy(char *filename1, char *filename2, int numOfRecords, int sizeOfRecords) {
    char buf[sizeOfRecords + 1];

    FILE *file1 = fopen(filename1, "r");
    FILE *file2 = fopen(filename2, "w");

    if (file1 && file2) {
        for (int i = 0; i < numOfRecords; ++i) {
            fread(buf, 1, (sizeOfRecords + 1), file1);
            if (i == numOfRecords - 1) fwrite(buf, 1, sizeOfRecords, file2);
            else fwrite(buf, 1, sizeOfRecords + 1, file2);
        }
        fclose(file1);
        fclose(file2);
    }
}

int checkStrings(char *first, char *second, int size) {
    for (int i = 0; i < size; ++i) {
        if (first[i] != second[i]) return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    struct tms tms1, tms2;

    if (argc > 7 || argc < 5) {
        printf("Wrong number of arguments\n");
        return 0;
    }

    if (argc == 7) {
        char *fun;
        fun = argv[1];
        if (checkStrings(fun, "copy", 4) == 1) {
            char *f1, *f2, *type;
            int numOfRecords, sizeOfRecords;

            f1 = argv[2];
            f2 = argv[3];

            numOfRecords = (int) strtol(argv[4], NULL, 10);
            sizeOfRecords = (int) strtol(argv[5], NULL, 10);
            if (numOfRecords <= 0 || sizeOfRecords <= 0) {
                printf("Invalid arguments\n");
                return 0;
            }

            type = argv[6];

            if (checkStrings(type, "sys", 3) == 1) {
                clock_t clktms1 = times(&tms1);
                syscopy(f1, f2, numOfRecords, sizeOfRecords);
                clock_t clktms2 = times(&tms2);
            }
            else if (checkStrings(type, "lib", 3) == 1) {
                clock_t clktms1 = times(&tms1);
                libcopy(f1, f2, numOfRecords, sizeOfRecords);
                clock_t clktms2 = times(&tms2);
            }
            else {
                printf("Invalid arguments\n");
            }
        }
        else printf("Invalid arguments\n");
    }

    if (argc == 6) {
        char *fun;
        fun = argv[1];
        if (checkStrings(fun, "sort", 4) == 1) {
            char *f1, *type;
            int numOfRecords, sizeOfRecords;

            f1 = argv[2];

            numOfRecords = (int) strtol(argv[3], NULL, 10);
            sizeOfRecords = (int) strtol(argv[4], NULL, 10);
            if (numOfRecords <= 0 || sizeOfRecords <= 0) {
                printf("Invalid arguments\n");
                return 0;
            }

            type = argv[5];

            if (checkStrings(type, "sys", 3) == 1) {
                clock_t clktms1 = times(&tms1);
                syssort(f1, numOfRecords, sizeOfRecords);
                clock_t clktms2 = times(&tms2);
            }
            else if (checkStrings(type, "lib", 3) == 1) {
                clock_t clktms1 = times(&tms1);
                libsort(f1, numOfRecords, sizeOfRecords);
                clock_t clktms2 = times(&tms2);
            }
            else {
                printf("Invalid arguments\n");
            }
        }
        else printf("Invalid arguments\n");
    }

    if (argc == 5) {
        char *fun;
        fun = argv[1];
        if (checkStrings(fun, "generate", 8) == 1) {
            char *f1;
            int numOfRecords, sizeOfRecords;

            f1 = argv[2];

            numOfRecords = (int) strtol(argv[3], NULL, 10);
            sizeOfRecords = (int) strtol(argv[4], NULL, 10);

            if (numOfRecords > 0 && sizeOfRecords > 0) generate(f1, numOfRecords, sizeOfRecords);
            else printf("Invalid arguments\n");
        }
        else printf("Invalid arguments\n");
        return 0;
    }

    if (argc == 6) printf("Pomiary czasu dla funkcji %s przy użyciu funkcji %s: \n", argv[1], argv[5]);
    else printf("Pomiary czasu dla funkcji %s przy użyciu funkcji %s: \n", argv[1], argv[6]);
    printf("Czas uzytkownika: %.10f, czas systemowy: %.10f\n", (double)(tms2.tms_utime - tms1.tms_utime)/sysconf(_SC_CLK_TCK), (double)(tms2.tms_stime - tms1.tms_stime)/sysconf(_SC_CLK_TCK));

    return 0;
}
