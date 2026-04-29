#include "mem.h"
#include "../misc/utils.h"
#include "../screen.h"

// Kernel pde, aligned on 4kb
// The page directory is formed of 1024 page tables (pde)
// The page table is formed of 1024 pages (pte)
static pde_t kernel_pde[1024] __attribute__((aligned(4096)));
static pte_t kernel_pte[256][1024] __attribute__((aligned(4096)));

static uint32_t *pagebitmap;

static void paging_load_directory(pde_t *pde)
{
    uint32_t p_pde = (uint32_t) pde - 0xC0000000;
    load_paging_directory(p_pde);
}

void init_paging(
    uint32_t kernel_start,
    uint32_t kernel_physical_start,
    uint32_t kernel_end,
    uint32_t kernel_physical_end
)
{
    // TODO: map precise area of kernel memory, not just the whole 1GB
    UNUSED(kernel_start);
    UNUSED(kernel_physical_start);
    UNUSED(kernel_end);
    UNUSED(kernel_physical_end);

    pagebitmap = &kernel_end;

    // We are bootstrapped to this point using the default 1mb page map
    // We want to setup our own which covers the whole of the kernel
    // First: We want to calculate the number of pages we need to map the kernel
    uint32_t p_count = kernel_physical_end / 4096 + 1;

    for (uint32_t i = 0; i < p_count; i++)
    {
        uint32_t pde_offset = i / 1024;
        uint32_t pte_offset = i % 1024;

        // phys addr = directory * 1024 + page * 4096 bytes
        uint32_t p_addr = i * 4096;
        kernel_pte[pde_offset][pte_offset] = p_addr | PTE_PRESENT | PTE_RW;

        if (pte_offset == 0) {
            uint32_t p_pt = (uint32_t) kernel_pte[pde_offset] - 0xC0000000;
            kernel_pde[pde_offset + 768] = p_pt | PTE_PRESENT | PTE_RW;
        }
    }

    paging_load_directory(kernel_pde);
}

/*
 * Allocates a page, returns the base address of the page 
 */
static uint32_t kalloc_frame()
{
    for (uint32_t i = 0; i < TOTAL_BITMAP_NUMBER; i++)
    {
        // If we have space in this bitmap
        if (pagebitmap[i] < MAX_UINT32_T) {
            uint32_t page_num = i * 32;

            for (uint32_t j = 0; j < 32; j++) {
                // This page is free if not marked already
                if (!((pagebitmap[i] >> j) & 0x1)) {
                    pagebitmap[i] |= (0x1 << j);
                    page_num += j;
                    return page_num * 4096;
                }
            }
        }
    }

    return PAGE_ALLOC_FAIL;
}


/*
 * Allocates a frame, maps it to a page
 */
uint32_t kalloc_page()
{

}

void kfree_page(uint32_t page)
{
    uint32_t page_number = page / 4096;

    uint32_t bitmap_num = page_number / 32;
    uint32_t offset = page_number % 32;

    pagebitmap[bitmap_num] &= (~(0x1 << offset));
}
