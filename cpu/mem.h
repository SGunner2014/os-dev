#include "../types.h"

#ifndef INCLUDE_MEM_H
#define INCLUDE_MEM_H

#define PTE_PRESENT     (1u << 0)
#define PTE_RW          (1u << 1)
#define PTE_USER        (1u << 2)
#define PTE_PWT         (1u << 3)
#define PTE_PCD         (1u << 4)
#define PTE_ACCESSED    (1u << 5)
#define PTE_DIRTY       (1u << 6)
#define PTE_PAT         (1u << 7)
#define PTE_GLOBAL      (1u << 8)

#define ADDRESS_MASK    0xFFFFF000

#define TOTAL_PAGE_NUMBER (1024 * 1024)
#define TOTAL_BITMAP_NUMBER (TOTAL_PAGE_NUMBER / 32)

extern void load_paging_directory(uint32_t p_pde);


/*
 * flags:
 * -- flags --
 * 0 -> Present
 * 1 -> read/write
 * 2 -> user/supervisor (1 = all, 0 = supervisor)
 * 3 -> PWT (write-through)
 * 4 -> PCD (cache disable)
 * 5 -> Accessed (has this page been read?)
 * 6 -> Dirty (has this page been written to?)
 * 7 -> PAT
 * -- flags_end --
 * 8 -> Global (1 = cpu not invalidate upon mov to cr3 instr.) relies on cr4[7] = 1
 * 9 .. 11 -> AVL (Ignored by cpu)
 * 12 .. 15 -> address low
 * -- address high --
 * 16 .. 31 -> address high
 */
typedef uint32_t pte_t;

/*
 * 0 -> Present
 * 1 -> read/write
 * 2 -> user/supervisor (1 = all, 0 = supervisor)
 * 3 -> PWT (write-through)
 * 4 -> PCD (cache disable)
 * 5 -> Accessed (has this page been read?)
 * 6 -> AVL
 * 7 -> Page Size
 * 8 .. 11 -> AVL
 * 12 .. 31 -> Address
 */
typedef uint32_t pde_t;

/*
 * Initialises system paging
 */
void init_paging(
    uint32_t kernel_start,
    uint32_t kernel_physical_start,
    uint32_t kernel_end,
    uint32_t kernel_physical_end
);

uint32_t kalloc_page();
uint32_t kfree_page(uint32_t);

#endif
