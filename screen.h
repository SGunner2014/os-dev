#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

// void write(char *buff, unsigned int len);
void printui(uint32_t i, uint32_t base);
void prints(char *buff);
void printc(char c);
void clear_screen();

#endif
