#include "mem.h"
#include "../misc/utils.h"

// Kernel pde, aligned on 4kb
// The page directory is formed of 1024 page tables (pde)
// The page table is formed of 1024 pages (pte)
static pde_t kernel_pde[1024] __attribute__((aligned(PAGE_SIZE)));
static pte_t kernel_pte[256][1024] __attribute__((aligned(PAGE_SIZE)));
// Last virtual address allocated
static uint32_t last_page_addr = 0;
static uint32_t *pagebitmap;

static void mark_page_as_occupied(uint32_t page)
{
    uint32_t bitmap_index = page / 32;
    uint32_t offset = page % 32;

    pagebitmap[bitmap_index] |= (0x1 << offset);
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
                    return page_num * PAGE_SIZE;
                }
            }
        }
    }

    return PAGE_ALLOC_FAIL;
}

/*
 * Takes in an allocated frame, and maps it to a virtual page
 */
static uint32_t kmap_frame(uint32_t frame_addr)
{
    // We need to find the first available page
    last_page_addr += PAGE_SIZE;
    uint32_t page_addr = last_page_addr;

    uint32_t pde_offset = (page_addr / PAGE_SIZE) / 1024;
    uint32_t pte_offset = ((page_addr - PHYS_OFFSET) / PAGE_SIZE) % 1024;

    // Map the page in the pte
    // Also, if the pde_offset == 0, then we need to map the pde as well
    kernel_pte[pde_offset - 768][pte_offset] = frame_addr | PTE_PRESENT | PTE_RW;

    if (pte_offset == 0) {
        uint32_t p_pt = (uint32_t) kernel_pte[pde_offset - 768] - PHYS_OFFSET;
        kernel_pde[pde_offset] = p_pt | PTE_PRESENT | PTE_RW;
    }

    return page_addr;
}


/*
 * Allocates a frame, maps it to a page
 */
uint32_t kalloc_page()
{
    uint32_t frame_addr = kalloc_frame();
    uint32_t virt_addr = kmap_frame(frame_addr);

    return virt_addr;
}

/*
 * Frees a previously allocated page
 */
void kfree_page(uint32_t address)
{
    // translate the virtual address to a physical one using the pt, pd
    uint32_t pde_offset = address / PAGE_SIZE / 1024;
    uint32_t pte_offset = ((address - PHYS_OFFSET) / PAGE_SIZE) % 1024;

    uint32_t p_addr = kernel_pte[pde_offset - 768][pte_offset] & ADDRESS_MASK;

    uint32_t page_number = p_addr / PAGE_SIZE;

    uint32_t bitmap_num = page_number / 32;
    uint32_t offset = page_number % 32;

    pagebitmap[bitmap_num] &= (~(0x1 << offset));
}

static void paging_load_directory(pde_t *pde)
{
    uint32_t p_pde = (uint32_t) pde - PHYS_OFFSET;
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
    uint32_t p_count = kernel_physical_end / PAGE_SIZE + 1;

    for (uint32_t i = 0; i < p_count; i++)
    {
        uint32_t pde_offset = i / 1024;
        uint32_t pte_offset = i % 1024;

        // phys addr = directory * 1024 + page * PAGE_SIZE bytes
        uint32_t p_addr = i * PAGE_SIZE;
        kernel_pte[pde_offset][pte_offset] = p_addr | PTE_PRESENT | PTE_RW;

        mark_page_as_occupied(i);

        if (pte_offset == 0) {
            uint32_t p_pt = (uint32_t) kernel_pte[pde_offset] - PHYS_OFFSET;
            kernel_pde[pde_offset + 768] = p_pt | PTE_PRESENT | PTE_RW;
        }

        last_page_addr = (PHYS_OFFSET) + (i * PAGE_SIZE);
    }

    paging_load_directory(kernel_pde);
}
