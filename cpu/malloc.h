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

typedef struct {
    Header base;
    Header *freep;
} Heap;

Header *kmorecore(uint32_t nunits, Heap* heap);
Header *morecore(uint32_t nunits, Heap* heap);

void *malloc(uint32_t size);
void free(void *ptr);


void *kmalloc(uint32_t size);
void kfree(void *ptr);

#endif
