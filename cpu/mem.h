
#ifndef INCLUDE_MEM_H
#define INCLUDE_MEM_H

#include "../types.h"
#include "multiboot.h"

#define PHYS_OFFSET     0xC0000000
#define PAGE_SIZE       0x1000 // 4KB pages

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

#define PAGE_ALLOC_FAIL 0xFFFFFFFF

extern void load_paging_directory(uint32_t p_pde);

typedef struct process Process;

typedef uint32_t pte_t;
typedef uint32_t pde_t;

/*
 * Initialises system paging
 */
void init_paging(
    uint32_t kernel_start,
    uint32_t kernel_physical_start,
    uint32_t kernel_end,
    uint32_t kernel_physical_end,
    multiboot_info_t *multiboot
);

uint32_t kalloc_page(uint32_t page_count);
void kfree_page(uint32_t);
uint32_t kvirt_to_phys(uint32_t virt);

uint32_t *create_page_directory();
uint32_t allocate_pages(uint32_t page_count, Process *process);
char is_virt_addr_mapped(uint32_t virt_addr);
void copy_mem(uint32_t *from, uint32_t *to, uint32_t size);
void copy_kernel_mappings(Process *process);

#endif
