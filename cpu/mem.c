#include "mem.h"
#include "../misc/utils.h"
#include "../screen.h"

// Kernel pde, aligned on 4kb
// The page directory is formed of 1024 page tables (pde)
// The page table is formed of 1024 pages (pte)
static pde_t kernel_pde[1024] __attribute__((aligned(4096)));
static pte_t kernel_pte[256][1024] __attribute__((aligned(4096)));

static uint32_t *pagebitmap;

uint32_t bitmap[32768];

void paging_load_directory(pde_t *pde)
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
    for (uint32_t pde_offset = 0; pde_offset < 256; pde_offset++)
    {
        for (uint32_t pte_offset = 0; pte_offset < 1024; pte_offset++)
        {
            uint32_t p_addr = ((pde_offset * 1024 + pte_offset) * 4096);
            kernel_pte[pde_offset][pte_offset] = p_addr | PTE_PRESENT | PTE_RW;
        }

        // Now write the page directories
        uint32_t p_pt = (uint32_t) kernel_pte[pde_offset] - 0xC0000000;
        kernel_pde[pde_offset + 768] = p_pt | PTE_PRESENT | PTE_RW;
    }

    paging_load_directory(kernel_pde);

    // Test the paging
    // CPU will page fault if messed up
    uint32_t *test = (uint32_t*) 0xD0000000;
    *test = 0xDEADBEEF;

    if (*test == 0xDEADBEEF) {
        prints("Successfully setup paging!\n");
    } else {
        prints("Failed to setup paging!\n");
    }
}
