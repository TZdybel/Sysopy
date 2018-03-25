//
// Created by tzdybel on 24.03.18.
//

#include <stdio.h>
#include <stdlib.h>

int main() {
    char *tab = (char *)calloc(100*1024*1024, sizeof(char));
    for(int i = 0; i<100*1024*1024; ++i) {
	tab[i] = 'a';
    }
    free(tab);
    return 0;
}
