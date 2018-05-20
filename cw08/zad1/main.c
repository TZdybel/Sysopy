#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <sys/times.h>
#include <unistd.h>

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(c,d) \
   ({ __typeof__ (c) _c = (c); \
       __typeof__ (d) _d = (d); \
     _c > _d ? _d : _c; })

int **I;
float **K;
int **J;

int W, H, M, c;
FILE *pgm;
FILE *filter;
FILE *result;

void closeAll(char *buf) {
    fclose(pgm);
    fclose(filter);
    fclose(result);
    free(buf);
}

void getParameters(char *buf) {
    if(fgets(buf, 1000, filter) != NULL) {
        c = (int)strtol(buf, NULL, 10);
    } else {
        printf("Something is wrong with filter file\n");
        closeAll(buf);
        exit(1);
    }
    for(int i = 0; i < 4; i++) {
        if(fgets(buf, 100, pgm) != NULL) {
            if(i == 1) {
                W = (int) strtol(strtok(buf, " \n\t\v\f\r"), NULL, 10);
                H = (int) strtol(strtok(NULL, " \n\t\v\f\r"), NULL, 10);
            } else if(i == 2) {
                M = (int) strtol(buf, NULL, 10);
            }
        } else {
            printf("Something is wrong with pgm file\n");
            closeAll(buf);
            exit(1);
        }
    }
    if (M > 255) {
        printf("Too big max value of pixel\n");
        closeAll(buf);
        exit(1);
    }
}

void getMatrixI(char *buf) {
    I = (int **)calloc(H, sizeof(int *));
    for (int i = 0; i < H; ++i) {
        I[i] = (int *)calloc(W, sizeof(int));
    }
    char *tmp = strtok(buf, " \n\t\v\f\r");
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            if(tmp == NULL) {
                if (fgets(buf, 100, pgm) == NULL) {
                    printf("Something is wrong with pgm file\n");
                    closeAll(buf);
                    free(tmp);
                    exit(1);
                }
                tmp = strtok(buf, " \n\t\v\f\r");
            }
            I[i][j] = (int)strtol(tmp, NULL, 10);
            tmp = strtok(NULL, " \n\t\v\f\r");
        }
    }
    free(tmp);
}

void getFilter(char *buf) {
    K = (float **)calloc(c, sizeof(float *));
    for (int i = 0; i < c; ++i) {
        K[i] = (float *)calloc(c, sizeof(float));
    }

    if(fgets(buf, 1000, filter) != NULL) {
        char *tmp = strtok(buf, " \n\t\v\f\r");
        for (int i = 0; i < c; ++i) {
            for (int j = 0; j < c; ++j) {
                if(tmp == NULL) {
                    if (fgets(buf, 1000, filter) == NULL) {
                        printf("Something is wrong with filter file\n");
                        closeAll(buf);
                        free(tmp);
                        exit(1);
                    }
                    tmp = strtok(buf, " \n\t\v\f\r");
                }
                K[i][j] = strtof(tmp, NULL);
                tmp = strtok(NULL, " \n\t\v\f\r");
            }
        }
        free(tmp);
    } else {
        printf("Something is wrong with filter file\n");
        closeAll(buf);
        exit(1);
    }
}

void *generatePicture(void *arg) {
    int *nums = (int *)arg;
    int cceil = (int)ceil(c/2);
    double value = 0;
    printf("Starting thread for columns from %d to %d\n", nums[0], nums[1]-1);
    for (int x = 0; x < H; ++x) {
        for (int y = nums[0]; y < nums[1]; ++y) {

            value = 0;
            for (int i = 1; i <= c; ++i) {
                for (int j = 1; j <= c; ++j) {
                    value += (I[min((H-1), max(0, (x-cceil+i)))][min((W-1), max(0, (y-cceil+j)))] * K[i-1][j-1]);
                }
            }
            J[x][y] = (int)round(value);
        }
    }
    free(nums);
}

void startThreads(int numOfThreads) {
    int numOfColumns = W/numOfThreads;
    pthread_t *threads = (pthread_t *)calloc(numOfThreads, sizeof(pthread_t));
    for (int i = 0; i < numOfThreads; ++i) {
        int *nums = (int *)calloc(2, sizeof(int));
        nums[0] = i*numOfColumns;
        if (i != numOfThreads-1) nums[1] = i*numOfColumns + numOfColumns;
        else nums[1] = W;
        pthread_create(&threads[i], NULL, generatePicture, (void *)nums);
    }
    for (int i = 0; i < numOfThreads; ++i) {
        pthread_join(threads[i], NULL);
    }
    free(threads);
}

void saveResult() {
    fprintf(result, "P2\n%d %d\n%d\n", W, H, M);
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            fprintf(result, "%d ", J[i][j]);
        }
        fprintf(result, "\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Wrong number of arguments\n");
        exit(1);
    }
    pgm = fopen(argv[2], "r");
    filter = fopen(argv[3], "r");
    result = fopen(argv[4], "w");
    if (pgm == NULL || filter == NULL || result == NULL) {
        printf("Wrong filepath\n");
        exit(1);
    }
    int numOfThreads = (int)strtol(argv[1], NULL, 10);
    char *buf = (char *)calloc(1000, sizeof(char));

    getParameters(buf);
    getMatrixI(buf);
    getFilter(buf);

    J = (int **)calloc(H, sizeof(int *));
    for (int i = 0; i < H; ++i) {
        J[i] = (int *)calloc(W, sizeof(int));
    }

    clock_t clk1, clk2;
    clk1 = times(NULL);
    startThreads(numOfThreads);
    clk2 = times(NULL);

    saveResult();
    double time = (double)(clk2 - clk1)/sysconf(_SC_CLK_TCK);
    printf("Real time elapsed: %.10f\n", time);;

    closeAll(buf);
    return 0;
}
