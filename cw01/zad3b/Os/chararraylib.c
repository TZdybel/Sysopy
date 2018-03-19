#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <zconf.h>
#include <math.h>
#include "chararraylib.h"


//zmienne globalne potrzebne do tablicy statycznej

char staticTab[10000][10000];     //przyjęte wymiary 10000x10000
int used[10000];                //"zajęte" blocki w tablicy statycznej, jeśli 0 to nie zajęte
int staticTabSize = 0;          //aktualne rozmiary tablicy:  ilość bloków
int sizeOfStaticBlocks = 0;                                 //wielkość bloków


//tablica dynamiczna


char **createTab(unsigned int size) {
    char **tab;
    tab = (char **)calloc(size, sizeof(char *));
    return tab;
}

void deleteTab(char **tab, unsigned int actual_size) {
    for (int i = 0; i < actual_size; ++i) {
        free(tab[i]);
    }
    free(tab);
}

void addBlock(char **tab, int size_of_tab, char text[], unsigned int size_of_block, int idx) {
    if (idx >= size_of_tab) {
        realloc(tab, (idx+1) * sizeof(char *));
    }
    tab[idx] = (char *)calloc(size_of_block, sizeof(char));
    for (int i = 0; i < size_of_block; ++i) {
        tab[idx][i] = text[i];
    }
}

void deleteBlock(char **tab, int idx) {
    free(tab[idx]);
}

int ASCIIsum (char **tab, unsigned int size_of_tab, int x, int y) {
    int idx = 0;
    int elem = (int)tab[x][y];
    int diff = INT_MAX;
    for (int i = 0; i < size_of_tab; ++i) {
        int sum = 0;
        if (tab[i] != NULL) {
            int tmp = strlen(tab[i]);
            for (int j = 0; j < tmp; ++j) {
                sum = sum + (int)tab[i][j];
            }
            if (abs(sum - elem) < diff) {
                diff = abs(sum - elem);
                idx = i;
            }
        }
    }
    return idx;
}


//tablica statyczna


char *createStaticTab(unsigned int size, unsigned int block_size) {
    staticTabSize = size;
    sizeOfStaticBlocks = block_size;
    for (int i = 0; i < size; ++i) {
        used[i] = 0;
    }
    return &staticTab[0][0];
}

void deleteStaticTab() {
    staticTabSize = 0;
    sizeOfStaticBlocks = 0;
}

void addStaticBlock(char text[], unsigned int size_of_block, int idx) {
    if (idx >= staticTabSize) {
        for (int i = staticTabSize; i < idx + 1; ++i) {
            used[i] = 0;
        }
        staticTabSize = idx + 1;
    }
    used[idx] = size_of_block;
    for (int i = 0; i < size_of_block; ++i) {
        staticTab[idx][i] = text[i];
    }
}

void deleteStaticBlock(int idx) {
    if (idx < staticTabSize) {
        used[idx] = 0;
        for (int i = 0; i < sizeOfStaticBlocks; ++i) {
            staticTab[idx][i] = ' ';
        }
    }
}

int ASCIIStaticSum(int x, int y) {
    int idx = 0;
    int elem = (int)staticTab[x][y];
    int diff = INT_MAX;
    for (int i = 0; i < staticTabSize; ++i) {
        int sum = 0;
        int tmp = used[i];
        for (int j = 0; j < tmp; ++j) {
            sum = sum + (int)staticTab[i][j];
        }
        if (abs(sum - elem) < diff) {
            diff = abs(sum - elem);
            idx = i;
        }
    }
    return idx;
}

void printStaticTab() {
    for (int j = 0; j < staticTabSize; ++j) {
        for (int i = 0; i < sizeOfStaticBlocks; ++i) {
            printf("%c ", staticTab[i][j]);
        }
        printf("\n");
    }
}
