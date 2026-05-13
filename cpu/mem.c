#include "mem.h"
#include "../misc/utils.h"
#include "process.h"


// Kernel pde, aligned on 4kb
// The page directory is formed of 1024 page tables (pde)
// The page table is formed of 1024 pages (pte)
static pde_t kernel_pde[1024] __attribute__((aligned(PAGE_SIZE)));
static pte_t kernel_pte[256][1024] __attribute__((aligned(PAGE_SIZE)));
// Last virtual address allocated
static uint32_t last_page_addr = 0;
// Bitmap to represent all page frames in memory
static uint32_t *framebitmap;

static void mark_frame_as_occupied(uint32_t page)
{
    uint32_t bitmap_index = page / 32;
    uint32_t offset = page % 32;

    framebitmap[bitmap_index] |= (0x1 << offset);
}

char is_virt_addr_mapped(uint32_t virt_addr)
{
    uint32_t pde_offset = virt_addr >> 22;
    uint32_t pte_offset = (virt_addr >> 12) & 0x3FF;

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
        kernel_pte[pde_offset - 768][pte_offset] = frame_addrs[i] | PTE_PRESENT | PTE_RW | PTE_USER;

        if (!(kernel_pde[pde_offset] & PTE_PRESENT)) {
            uint32_t p_pt = (uint32_t) kernel_pte[pde_offset - 768] - PHYS_OFFSET;
            kernel_pde[pde_offset] = p_pt | PTE_PRESENT | PTE_RW | PTE_USER;
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
    uint32_t p_pde = (uint32_t) pde - PHYS_OFFSET;
    load_paging_directory(p_pde);
}

/**
 * Given the kernel physical end and a GRUB v1 multiboot structure, this
 * computes the first page-aligned address of free space.
 *
 * Returns a physical address
 */
static uint32_t calculate_free_space_beginning(uint32_t kernel_phys_end, multiboot_info_t *multiboot)
{
    uint32_t addr = kernel_phys_end;

    multiboot_module_t *mod;
    uint32_t i;

    // Take the end address of each module in case it's larger than the end of the kernel physical
    for (i = 0, mod = (multiboot_module_t*) (uint32_t) (multiboot->mods_addr + PHYS_OFFSET); i < multiboot->mods_count; i++, mod++)
    {
        addr = mod->mod_end > addr ? mod->mod_end : addr;

        uint32_t mod_end = ((uint32_t) mod) - PHYS_OFFSET;
        mod_end = mod_end + sizeof(multiboot_module_t);
        addr = mod_end > addr ? mod_end : addr;
    }

    uint32_t multiboot_end = (((uint32_t) multiboot) - PHYS_OFFSET) + sizeof(multiboot_info_t);
    addr = multiboot_end > addr ? multiboot_end : addr;

    // Now, make it page-aligned
    return addr % PAGE_SIZE == 0
        ? addr
        : ((addr / PAGE_SIZE) + 1) * PAGE_SIZE;
}

void init_paging(
    uint32_t kernel_start,
    uint32_t kernel_physical_start,
    uint32_t kernel_end,
    uint32_t kernel_physical_end,
    multiboot_info_t *multiboot
)
{
    UNUSED(kernel_start);
    UNUSED(kernel_end);
    UNUSED(kernel_physical_start);

    uint32_t phys_end = calculate_free_space_beginning(kernel_physical_end, multiboot);
    framebitmap = (uint32_t*) (phys_end + PHYS_OFFSET);

    // We are bootstrapped to this point using the default 1mb page map
    // We want to setup our own which covers the whole of the kernel
    // First: We want to calculate the number of pages we need to map the kernel
    uint32_t p_count = phys_end / PAGE_SIZE + 2;

    for (uint32_t i = 0; i < p_count; i++)
    {
        uint32_t pde_offset = i / 1024;
        uint32_t pte_offset = i % 1024;

        // phys addr = directory * 1024 + page * PAGE_SIZE bytes
        uint32_t p_addr = i * PAGE_SIZE;
        kernel_pte[pde_offset][pte_offset] = p_addr | PTE_PRESENT | PTE_RW | PTE_USER;

        if (!(kernel_pde[pde_offset] & PTE_PRESENT)) {
            uint32_t p_pt = (uint32_t) kernel_pte[pde_offset] - PHYS_OFFSET;
            kernel_pde[pde_offset + 768] = p_pt | PTE_PRESENT | PTE_RW | PTE_USER;
        }

        last_page_addr = (PHYS_OFFSET) + (i * PAGE_SIZE);
    }

    paging_load_directory(kernel_pde);

    // Now, we can go back and mark the pages as occupied
    for (uint32_t i = 0; i < p_count; i++)
    {
        mark_frame_as_occupied(i);
    }
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

/**
 * Maps a physical frame to a virtual address in a process' page directory
 * We figure out the page directory index and the page table index
 * If the page directory entry doesn't yet exist, we need to allocate a new frame
 * for the page table for this directory entry.
 * This involves allocating a new frame, mapping it to a virtual address, and then
 * adding the entry in the page directory to point to the new physical address.
 *
 * This new frame will also need to be accessible from the kernel, so we basically
 * just 'identity map' it with the physical offset for the kernel included.
 *
 * Note: not a true identity map, as the phys != virt
 */
void map_page(pde_t *pd, uint32_t virt, uint32_t phys, uint32_t flags)
{
    uint32_t pde_offset = virt >> 22;
    uint32_t pte_offset = (virt >> 12) & 0x3FF; // 0x3FF = 10 bits

    pte_t *pt_virt = NULL;

    // If we don't already have this page directory entry mapped to a
    // page table, we need to allocate a new frame and map it
    if (!(pd[pde_offset] & PTE_PRESENT)) {
        // Allocate a page and map
        // This means it's already present in kernel_pde, kernel_pte
        uint32_t page = kalloc_page(1);

        // Translate the virtual address given to use by the kernel
        // then set the directory entry to the (phys) addr of the new frame
        uint32_t phys_page = kvirt_to_phys(page);
        pd[pde_offset] = phys_page | PTE_PRESENT | PTE_RW | PTE_USER;

        pt_virt = (pte_t*) page;

        // Next, zero out the page table
        for (uint32_t i = 0; i < PAGE_SIZE / sizeof(pte_t); i++) {
            pt_virt[i] = 0;
        }
    } else {
        uint32_t pt_phys = pd[pde_offset] & ~0xFFF;
        pt_virt = (pte_t*) (pt_phys + PHYS_OFFSET);
    }

    // Now we can finally map the virtual addr to the physical one
    pt_virt[pte_offset] = phys | flags;
}

/**
 * Takes in a list of physical frames, and maps them to contiguous memory locations
 * within the specified page directory
 *
 * Returns the virtual address of the first page
 */
uint32_t map_frames(uint32_t *frames, uint32_t page_count, uint32_t* last_virt_addr, pde_t *pd)
{
    uint32_t start_addr = NULL;

    for (uint32_t i = 0; i < page_count; i++)
    {
        *last_virt_addr += 4096;
        map_page(pd, *last_virt_addr, frames[i], PTE_PRESENT | PTE_RW | PTE_USER);

        if (start_addr == NULL)
            start_addr = *last_virt_addr;
    }

    return start_addr;
}

/**
 * Allocate a new physical frame
 */
uint32_t allocate_frame()
{
    uint32_t addr = kalloc_frame();
    return addr;
}

/**
 * Translate a kernel virtual address to physical
 */
uint32_t kvirt_to_phys(uint32_t virt)
{
    uint32_t pde_offset = virt >> 22;
    uint32_t pte_offset = (virt >> 12) & 0x3FF;

    // Ditch the flags, return the address only
    return kernel_pte[pde_offset - 768][pte_offset] & ~0xFFF;
}

/**
 * Allocate page_count pages
 *
 * This allocates the required number of frames, and then maps them to
 * contiguous virtual addresses
 *
 * Retruns the virtual address of the first page.
 */
uint32_t allocate_pages(uint32_t page_count, Process *process)
{
    uint32_t frames[page_count];
    for (uint32_t i = 0; i < page_count; i++)
    {
        frames[i] = allocate_frame();
    }

    uint32_t virt_addr = map_frames(
        frames,
        page_count,
        &process->last_virt_addr,
        process->page_directory_virt
    );

    return virt_addr;
}

void copy_mem(uint32_t *from, uint32_t *to, uint32_t size)
{
    uint8_t *cto = (uint8_t*) to;
    uint8_t *cfrom = (uint8_t*) from;

    for (uint32_t i = 0; i < size; i++)
    {
        cto[i] = cfrom[i];
    }
}

void copy_kernel_mappings(Process *process)
{
    for (uint32_t i = 768; i < 1024; i++)
    {
        process->page_directory_virt[i] = kernel_pde[i];
    }
}
