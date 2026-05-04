#ifndef INCLUDE_MALLOC_H
#define INCLUDE_MALLOC_H

#include "../types.h"

typedef int32_t Align;

union header {
    struct {
        union header *ptr;
        uint32_t size;
    } s;
    Align x;
};

typedef union header Header;

Header *morecore(uint32_t nunits);
void *malloc(uint32_t size);
void free(void *ptr);

#endif
