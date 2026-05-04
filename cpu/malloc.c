#include "malloc.h"
#include "mem.h"
#include "../types.h"
#include "../misc/utils.h"

static Header base;
static Header *freep;

Header *morecore(uint32_t nunits)
{
    UNUSED(nunits);

    Header *p = (Header *) kalloc_page();
    uint32_t p_size = PAGE_SIZE;

    p->s.size = PAGE_SIZE / sizeof(Header);
    free((void*) (p + 1));
    return freep;
}

void *malloc(uint32_t size)
{
    uint32_t pages = size / PAGE_SIZE;

    // TEMPORARY - NEED TO REWORK PAGE ALLOC
    if (pages > 1) {
        return 0;
    }

    // List starts as empty if we haven't allocated any memory yet
    if (freep == NULL) {
        base.s.size = 0;
        base.s.ptr = &base;
        freep = &base;
    }

    uint32_t units = (size + sizeof(Header) - 1) / sizeof(Header) + 1;
    Header *p, *prevp;

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= units) {
            if (p->s.size == units) {
                prevp->s.ptr = p->s.ptr;
            } else {
                p->s.size -= units;
                p += p->s.size;
                p->s.size = units;
            }
            freep = prevp;
            return (void*)(p + 1);
        }
        if (p == freep) {
            if ((p = morecore(units)) == NULL) {
                return NULL;
            }
        }
    }
}

void free(void *ap)
{
    Header *bp, *p;

    bp = (Header*) ap - 1; // Point to block header
    for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr) {

    }
}
