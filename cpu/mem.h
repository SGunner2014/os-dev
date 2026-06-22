#ifndef INCLUDE_MEM_H
#define INCLUDE_MEM_H

#include "../types.h"
#include "process.h"

#define PAGE_SIZE 0x1000
#define PHYS_OFFSET 0xC0000000

#define TOTAL_PAGE_NUMBER (1024 * 1024)
#define TOTAL_BITMAP_NUMBER (TOTAL_PAGE_NUMBER / 32)

#define FRAME_ALLOC_FAIL 0xFFFFFFFF

#define PTE_PRESENT (1u << 0)
#define PTE_RW (1u << 1)
#define PTE_USER (1u << 2)
#define PTE_PWT (1u << 3)
#define PTE_PCD (1u << 4)
#define PTE_ACCESSED (1u << 5)
#define PTE_DIRTY (1u << 6)
#define PTE_PAT (1u << 7)
#define PTE_GLOBAL (1u << 8)

extern void load_paging_directory(uint32_t p_pde);

typedef uint32_t pte_t;
typedef uint32_t pde_t;

/*
 * Kernel Functions
 */

// Generic
void k_init_paging(
    //   uint32_t kernel_start,
    //   uint32_t kernel_physical_start,
    //   uint32_t kernel_end,
    //   uint32_t kernel_physical_end,
    //   multiboot_info_t *multiboot
    uint32_t kernel_physical_end);
void k_copy_kernel_mappings(pde_t *pd);

// Frames
uint32_t k_alloc_free_frame();
void k_mark_frame_occupied(uint32_t frame_no);
void k_free_frame(uint32_t frame_no);
bool k_is_frame_occupied(uint32_t frame_no);

// Pages
// kalloc - always returns virtual addresses, physical should be obtained with
// k_virt_to_phys
uint32_t *k_alloc_page(uint32_t *vaddr); // Returns the virtual address
                                         // allocated, can pass NULL as arg
void k_free_page(uint32_t *vaddr);
uint32_t *k_virt_to_phys(uint32_t *vaddr);
// uint32_t *k_phys_to_virt(uint32_t *paddr);
bool k_is_virt_allocated(uint32_t *vaddr);

/*
 * Process Functions
 */

// Pages
// alloc - always returns virtual addresses, physical should be obtained with
// virt_to_phys
uint32_t *
alloc_page(pde_t *pd, uint32_t *vaddr, uint32_t *last_proc_page_addr,
           uint32_t *virtual_pde); // Returns virt addr, can pass NULL as arg
void free_page(uint32_t *vaddr, uint32_t *virtual_pde);
uint32_t *virt_to_phys(uint32_t *virtual_pde, uint32_t *vaddr);
// uint32_t *phys_to_virt ?
bool is_virt_allocated(uint32_t *virtual_pde, uint32_t *vaddr);
pde_t *create_page_directory();
pde_t *create_virtual_page_directory();
uint32_t *map_virt_range(pde_t *pd, pde_t *virt_pd, uint32_t *vaddr,
                         uint32_t *paddr, uint32_t size);

/*
 * Utilities
 */
void copy_mem(uint32_t *from, uint32_t *to, uint32_t size);
bool is_page_aligned(uint32_t *addr);
uint32_t *get_page_aligned(uint32_t *addr);

#endif
