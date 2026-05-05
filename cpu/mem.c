#include "mem.h"
#include "../misc/utils.h"
#include "../screen.h"


// Kernel pde, aligned on 4kb
// The page directory is formed of 1024 page tables (pde)
// The page table is formed of 1024 pages (pte)
static pde_t kernel_pde[1024] __attribute__((aligned(PAGE_SIZE)));
static pte_t kernel_pte[256][1024] __attribute__((aligned(PAGE_SIZE)));
// Last virtual address allocated
static uint32_t last_page_addr = 0;
// Bitmap to represent all page frames in memory
static uint32_t *framebitmap;

static void mark_page_as_occupied(uint32_t page)
{
    uint32_t bitmap_index = page / 32;
    uint32_t offset = page % 32;

    framebitmap[bitmap_index] |= (0x1 << offset);
}

static char is_virt_mapped(uint32_t virt_addr)
{
    uint32_t pde_offset = virt_addr >> 22;
    uint32_t pte_offset = (virt_addr >> 12) & 0x3FF;

    UNUSED(pte_offset);

    if (kernel_pde[pde_offset] & PTE_PRESENT)
    {
        if (kernel_pte[pde_offset - 768][pte_offset] & PTE_PRESENT)
        {
            return 1;
        }
    }

    return 0;
}

/*
 * Allocates a page, returns the base address of the page
 */
static uint32_t kalloc_frame()
{
    for (uint32_t i = 0; i < TOTAL_BITMAP_NUMBER; i++)
    {
        // If we have space in this bitmap
        if (framebitmap[i] < MAX_UINT32_T) {
            uint32_t page_num = i * 32;

            for (uint32_t j = 0; j < 32; j++) {
                // This page is free if not marked already
                if (!((framebitmap[i] >> j) & 0x1)) {
                    framebitmap[i] |= (0x1 << j);
                    page_num += j;
                    return page_num * PAGE_SIZE;
                }
            }
        }
    }

    return PAGE_ALLOC_FAIL;
}

void kalloc_frames(uint32_t *frames, uint32_t frame_count)
{
    for (uint32_t i = 0; i < frame_count; i++)
    {
        frames[i] = kalloc_frame();
    }
}

static uint32_t kmap_frames(uint32_t *frame_addrs, uint32_t frame_count)
{
    uint32_t addr = NULL;

    for (uint32_t i = 0; i < frame_count; i++) {
        last_page_addr += PAGE_SIZE;
        uint32_t page_addr = last_page_addr;

        if (addr == NULL)
            addr = page_addr;

        uint32_t pde_offset = (page_addr / PAGE_SIZE) / 1024;
        uint32_t pte_offset = ((page_addr - PHYS_OFFSET) / PAGE_SIZE) % 1024;

        // Map the page in the pte
        // Also, if the pde_offset == 0, then we need to map the pde as well
        kernel_pte[pde_offset - 768][pte_offset] = frame_addrs[i] | PTE_PRESENT | PTE_RW;

        if (!(kernel_pde[pde_offset] & PTE_PRESENT)) {
            uint32_t p_pt = (uint32_t) kernel_pte[pde_offset - 768] - PHYS_OFFSET;
            kernel_pde[pde_offset] = p_pt | PTE_PRESENT | PTE_RW;
        }

        uint32_t *n_addr = (uint32_t*) page_addr;
        for (uint32_t i = 0; i < PAGE_SIZE / sizeof(uint32_t); i++)
        {
            n_addr[i] = 0;
        }
    }

    return addr;
}


/*
 * Allocates a frame, maps it to a page
 */
uint32_t kalloc_page(uint32_t page_count)
{
    uint32_t frames[page_count];

    kalloc_frames(frames, page_count);

    uint32_t virt_addr = kmap_frames(frames, page_count);

    prints("Mapped phys: ");
    char buff[256];
    itoa(frames[0], buff, 16);
    prints(buff);
    prints("\n");

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

    framebitmap[bitmap_num] &= (~(0x1 << offset));
}

static void paging_load_directory(pde_t *pde)
{
    char buff[256];
    itoa((uint32_t) pde, buff, 16);
    prints("Loading pd at: ");
    prints(buff);
    prints("\n");

    char is_present = is_virt_mapped((uint32_t) pde);
    itoa((uint32_t) is_present, buff, 2);
    prints("Is present: ");
    prints(buff);
    prints("\n");

    // for (;;) ;

    uint32_t p_pde = (uint32_t) pde - PHYS_OFFSET;
    load_paging_directory(p_pde);

    // for (;;) ;
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

    framebitmap = (uint32_t*) kernel_end;

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

        if (!(kernel_pde[pde_offset] & PTE_PRESENT)) {
            uint32_t p_pt = (uint32_t) kernel_pte[pde_offset] - PHYS_OFFSET;
            kernel_pde[pde_offset + 768] = p_pt | PTE_PRESENT | PTE_RW;
        }

        last_page_addr = (PHYS_OFFSET) + (i * PAGE_SIZE);
    }

    paging_load_directory(kernel_pde);
}

/**
 * Creates a new page directory for a process and maps the kernel
 */
uint32_t *create_page_directory()
{
    // Allocate and map a new page on the kernel space for the page directory
    pde_t *new_pd = (pde_t*) kalloc_page(1);

    // For the new page directory, each kernel page should also be mapped
    for (uint32_t i = 768; i < 1024; i++)
    {
        new_pd[i] = kernel_pde[i];
    }

    return new_pd;
}

void map_page(uint32_t *pd, uint32_t virt, uint32_t phys, uint32_t flags)
{
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF; // 0x3FF = 10 bits

    // We need to map this page directory entry first
    if (!(pd[pd_index] & PTE_PRESENT)) {
        uint32_t frame = kalloc_frame();
        pd[pd_index] = frame | PTE_PRESENT | PTE_RW;

        uint32_t *pt_virt = (uint32_t*) (frame + PHYS_OFFSET);
        for (uint32_t i = 0; i < PAGE_SIZE / sizeof(uint32_t); i++)
        {
            pt_virt[i] = 0;
        }
    }

    // Previously we had the physical address of the new frame
    // Now we need to get the virtual address so we can add our entry
    // for the page being mapped
    uint32_t *pt_addr_phys = (uint32_t*) (pd[pd_index] & ~0xFFF);
    uint32_t *pt_addr_virt =  (uint32_t*)((uint32_t) pt_addr_phys + PHYS_OFFSET);
    pt_addr_virt[pt_index] = phys | flags;
}

uint32_t map_frames(uint32_t *frames, uint32_t page_count, uint32_t* last_virt_addr, uint32_t *pd)
{
    uint32_t start_addr = NULL;

    for (uint32_t i = 0; i < page_count; i++)
    {
        *last_virt_addr += 4096;
        prints("here\n");
        map_page(pd, *last_virt_addr, frames[i], PTE_PRESENT | PTE_RW);
        prints("mapped page\n");

        if (start_addr == NULL)
            start_addr = *last_virt_addr;
    }

    return start_addr;
}

uint32_t allocate_frame()
{
    uint32_t addr = kalloc_frame();
    return addr;
}

uint32_t allocate_pages(uint32_t page_count, Process *process)
{
    uint32_t frames[page_count];
    for (uint32_t i = 0; i < page_count; i++)
    {
        frames[i] = allocate_frame();
    }

    prints("got frames\n");

    uint32_t virt_addr = map_frames(frames, page_count, &process->last_virt_addr, process->page_directory_virt);

    prints("Mapped frames\n");

    return virt_addr;
}
