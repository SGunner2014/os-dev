#include "../types.h"

#ifndef INCLUDE_UTILS_H
#define INCLUDE_UTILS_H

#define UNUSED(x) (void)(x)
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define KASSERT(cond) \
    do { \
        if (!(cond)) { \
            k_panic("KASSERT failed: " #cond " at " __FILE__ ":" STRINGIFY(__LINE__)); \
        } \
    } while (0)

unsigned int strlen(const char*);
void strrev(char *buff);
void itoa(const int32_t i, char *buff, const uint32_t base);
void printui(uint32_t i);
char *strcpy(char *buff1, const char *buff2);
void format_memory_str(uint32_t kb, char *buff);
void memset(void *addr, uint32_t value, uint32_t size);
void k_panic(char *err);

#endif
