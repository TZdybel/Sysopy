//
// Created by Tomek on 13.03.2018.
//

#ifndef SYSOPY1_CHARARRAYLIB_H
#define SYSOPY1_CHARARRAYLIB_H

#endif //SYSOPY1_CHARARRAYLIB_H

char **createTab(unsigned int size);
void deleteTab(char **tab, unsigned int actual_size);
void addBlock(char **tab, int size_of_tab, char text[], unsigned int size_of_block, int idx);
void deleteBlock(char **tab, int idx);
int ASCIIsum (char **tab, unsigned int size_of_tab, int x, int y);

char* createStaticTab(unsigned int size, unsigned int block_size);
void deleteStaticTab();
void addStaticBlock(char text[], unsigned int size_of_block, int idx);
void deleteStaticBlock(int idx);
int ASCIIStaticSum(int x, int y);
void printStaticTab();
