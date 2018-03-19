#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include "chararraylib.h"



int getTabSize() {
    int size = 0;
    printf("Enter tab size: ");
    scanf("%d", &size);
    return size;
}

int getBlockSize() {
    int block_size = 0;
    printf("Enter block size: ");
    scanf("%d", &block_size);
    return block_size;
}

int isStatic() {
    printf("If you want tab to be static, enter 'y' or 'Y': ");
    char type;
    scanf("%c", &type);
    if (type == 'y' || type == 'Y') return 1;
    else return 0;
}

void generateData(char text[], int block_size) {
    char set[53] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < block_size; ++i) {
        text[i] = set[rand()%52];
    }
}

void dynamicTableTests(int size, int block_size) {
    srand(time(NULL));
    clock_t first, second, third, fourth, fifth;
    struct tms tms1;
    char text[block_size];

    clock_t clktms1 = times(&tms1);
    first = clock();
    char **tab = createTab(size);
    for (int i = 0; i < size; ++i) {
        generateData(text, block_size);
        addBlock(tab, size, text, block_size, i);
    }

    struct tms tms2;
    clock_t clktms2 = times(&tms2);
    second = clock();
    int idx = ASCIIsum(tab, size, 0, 0);

    struct tms tms3;
    clock_t clktms3 = times(&tms3);
    third = clock();
    for (int i = 0; i < size/2; ++i) {
        deleteBlock(tab, i);
    }
    for (int i = 0; i < size/2; ++i) {
        generateData(text, block_size);
        addBlock(tab, size, text, block_size, i);
    }

    struct tms tms4;
    clock_t clktms4 = times(&tms4);
    fourth = clock();
    for (int i = 0; i < size/2; ++i) {
        deleteBlock(tab, i);
        generateData(text, block_size);
        addBlock(tab, size, text, block_size, i);
    }

    struct tms tms5;
    clock_t clktms5 = times(&tms5);
    fifth = clock();

    double time1, time2, time3, time4;
    time1 = (double)(second - first)/CLOCKS_PER_SEC;
    time2 = (double)(third - second)/CLOCKS_PER_SEC;
    time3 = (double)(fourth - third)/CLOCKS_PER_SEC;
    time4 = (double)(fifth - fourth)/CLOCKS_PER_SEC;

    printf("Wyniki dla tablicy dynamicznej o wymiarach %i x %i\n", size, block_size);
    printf("Tworzenie tablicy - czas rzeczywisty: %.10f, czas uzytkownika: %.10f, czas systemowy: %.10f\n", time1, (double)(tms2.tms_utime - tms1.tms_utime)/sysconf(_SC_CLK_TCK), (double)(tms2.tms_stime - tms1.tms_stime)/sysconf(_SC_CLK_TCK));
    printf("Wyszukiwanie podobnego elementu - czas rzeczywisty: %.10f, czas uzytkownika: %.10f, czas systemowy: %.10f\n", time2, (double)(tms3.tms_utime - tms2.tms_utime)/sysconf(_SC_CLK_TCK), (double)(tms3.tms_stime - tms2.tms_stime)/sysconf(_SC_CLK_TCK));
    printf("Usunięcie blokow i wstawienie w ich miejsce nowych - czas rzeczywisty: %.10f, czas uzytkownika: %.10f, czas systemowy: %.10f\n", time3, (double)(tms4.tms_utime - tms3.tms_utime)/sysconf(_SC_CLK_TCK), (double)(tms4.tms_stime - tms3.tms_stime)/sysconf(_SC_CLK_TCK));
    printf("Usuwanie i wstawianie blokow na przemian - czas rzeczywisty: %.10f, czas uzytkownika: %.10f, czas systemowy: %.10f\n", time4, (double)(tms5.tms_utime - tms4.tms_utime)/sysconf(_SC_CLK_TCK), (double)(tms5.tms_stime - tms4.tms_stime)/sysconf(_SC_CLK_TCK));
}

void staticTableTests(int size, int block_size) {
    srand(time(NULL));
    clock_t first, second, third, fourth, fifth;
    struct tms tms1;
    char text[block_size];

    clock_t clktms1 = times(&tms1);
    first = clock();
    char *tab = createStaticTab(size, block_size);
    for (int i = 0; i < size; ++i) {
        generateData(text, block_size);
        addStaticBlock(text, block_size, i);
    }

    struct tms tms2;
    clock_t clktms2 = times(&tms2);
    second = clock();
    int idx = ASCIIStaticSum(0, 0);

    struct tms tms3;
    clock_t clktms3 = times(&tms3);
    third = clock();
    for (int i = 0; i < size/2; ++i) {
        deleteStaticBlock(i);
    }
    for (int i = 0; i < size/2; ++i) {
        generateData(text, block_size);
        addStaticBlock(text, block_size, i);
    }

    struct tms tms4;
    clock_t clktms4 = times(&tms4);
    fourth = clock();
    for (int i = 0; i < size/2; ++i) {
        deleteStaticBlock(i);
        generateData(text, block_size);
        addStaticBlock(text, block_size, i);
    }

    struct tms tms5;
    clock_t clktms5 = times(&tms5);
    fifth = clock();

    double time1, time2, time3, time4;
    time1 = (double)(second - first)/CLOCKS_PER_SEC;
    time2 = (double)(third - second)/CLOCKS_PER_SEC;
    time3 = (double)(fourth - third)/CLOCKS_PER_SEC;
    time4 = (double)(fifth - fourth)/CLOCKS_PER_SEC;


    printf("Wyniki dla tablicy statycznej o wymiarach %i x %i\n", size, block_size);
    printf("Tworzenie tablicy - czas rzeczywisty: %.10f, czas uzytkownika: %.10f, czas systemowy: %.10f\n", time1, (double)(tms2.tms_utime - tms1.tms_utime)/sysconf(_SC_CLK_TCK), (double)(tms2.tms_stime - tms1.tms_stime)/sysconf(_SC_CLK_TCK));
    printf("Wyszukiwanie podobnego elementu - czas rzeczywisty: %.10f, czas uzytkownika: %.10f, czas systemowy: %.10f\n", time2, (double)(tms3.tms_utime - tms2.tms_utime)/sysconf(_SC_CLK_TCK), (double)(tms3.tms_stime - tms2.tms_stime)/sysconf(_SC_CLK_TCK));
    printf("Usunięcie blokow i wstawienie w ich miejsce nowych - czas rzeczywisty: %.10f, czas uzytkownika: %.10f, czas systemowy: %.10f\n", time3, (double)(tms4.tms_utime - tms3.tms_utime)/sysconf(_SC_CLK_TCK), (double)(tms4.tms_stime - tms3.tms_stime)/sysconf(_SC_CLK_TCK));
    printf("Usuwanie i wstawianie blokow na przemian - czas rzeczywisty: %.10f, czas uzytkownika: %.10f, czas systemowy: %.10f\n", time4, (double)(tms5.tms_utime - tms4.tms_utime)/sysconf(_SC_CLK_TCK), (double)(tms5.tms_stime - tms4.tms_stime)/sysconf(_SC_CLK_TCK));

}

int main() {
    int ifStatic = isStatic();
    int size = getTabSize();
    int block_size = getBlockSize();

    if (ifStatic == 1) staticTableTests(size, block_size);
    else dynamicTableTests(size, block_size);
    return 0;
}

