#include "malloc.h"
#include "mem.h"
#include "../types.h"
#include "../misc/utils.h"
#include "process.h"
#include "../screen.h"

// static Header base;
// static Header *freep = NULL;

static Heap kernel_heap = {
    .freep = NULL
};

/**
 * Morecore, but for kernel space
 */
Header *kmorecore(uint32_t nunits, Heap *heap)
{
    prints("Got to kmorecore");
    uint32_t page_count = ((nunits * sizeof(Header)) / PAGE_SIZE) + 1;
    Header *p = (Header *) kalloc_page(page_count);

    prints("ass");
    p->s.size = (page_count * PAGE_SIZE) / sizeof(Header);
    kfree((void*) (p + 1));

    prints("Before return");
    return heap->freep;
}

/**
 * Morecore, but for user space
 */
Header *morecore(uint32_t nunits, Heap *heap)
{
    Process *process = get_current_process();

    uint32_t page_count = ((nunits * sizeof(Header)) / PAGE_SIZE) + 1;
    prints("Before allocate pages\n");
    Header *p = (Header*) allocate_pages(page_count, process);

    prints("After allocate pages\n");

    p->s.size = (page_count * PAGE_SIZE) / sizeof(Header);
    free((void*) (p + 1));
    return heap->freep;
}

void *_malloc(uint32_t size, Heap *heap, Header* (*alloc_fn)(uint32_t, Heap*))
{
    uint32_t pages = size / PAGE_SIZE;
    uint32_t units = (size + sizeof(Header) - 1) / sizeof(Header) + 1;
    Header *p, *prevp;

    // TEMPORARY - NEED TO REWORK PAGE ALLOC
    if (pages > 1) {
        return 0;
    }

    // List starts as empty if we haven't allocated any memory yet
    if ((prevp = heap->freep) == NULL) {
        heap->base.s.ptr = heap->freep = prevp = &heap->base;
        heap->base.s.size = 0;
    }

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= units) {
            prints("1");
            if (p->s.size == units) {
                prevp->s.ptr = p->s.ptr;
            } else {
                p->s.size -= units;
                prints("2");
                p += p->s.size;
                prints("3");
                p->s.size = units;
                prints("4");
            }
            heap->freep = prevp;
            prints("5");
            return (void*)(p + 1);
        }
        if (p == heap->freep) {
            prints("Before alloc fn\n");
            if ((p = alloc_fn(units, heap)) == NULL) {
                return NULL;
            }
        }
    }
}

void *kmalloc(uint32_t size)
{
    return _malloc(size, &kernel_heap, kmorecore);
}

void *malloc(uint32_t size)
{
    Process *current_process = get_current_process();
    prints("Got current process\n");
    return _malloc(size, &current_process->heap, morecore);
}

void _free(void *ap, Heap *heap)
{
    Header *bp, *p;

    bp = (Header*) ap - 1; // Point to block header
    // so keep navigating until bp is bigger than p and bp is less then the next elem of p
    // This indicates that the current block fits in a gap we've found between two elements
    for (p = heap->freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {
        // If the block fits in either before the existing circular linked list
        // or after the last element in the linked list
        if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
            break; // block freed at start or end of arena
    }

    // If the end of the current block is the beginning of another block
    if (bp + bp->s.size == p->s.ptr) {
        bp->s.size += p->s.ptr->s.size;
        bp->s.ptr = p->s.ptr->s.ptr;
    } else { // Otherwise, the end of this block will point to the next one
        bp->s.ptr = p->s.ptr;
    }

    // If the start of the current block is the end of another block
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.ptr = bp->s.ptr;
    } else { // Otherwise, the end of the previous block will point to this one
        p->s.ptr = bp;
    }

    heap->freep = p;
}

void kfree(void *ap)
{
    prints("Got to kfree");
    _free(ap, &kernel_heap);
    prints("After kfree");
}

void free(void *ap)
{
    Process *current_process = get_current_process();
    _free(ap, &current_process->heap);
}
