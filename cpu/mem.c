#include "mem.h"
#include "../drivers/vga.h"
#include "../include/errno.h"
#include "../misc/utils.h"
#include "../types.h"

// Kernel directory
static pde_t kernel_pde[1024] __attribute__((aligned(PAGE_SIZE)));
static pte_t kernel_pte[256][1024] __attribute__((aligned(PAGE_SIZE)));
// Pde, but the addresses are virtual
static uint32_t kernel_virtual_pde[1024];

static uint32_t last_page_addr = 0;

static uint32_t *framebitmap;

/*
 * Utility Functions
 */

/**
 * Returns true if the addr is page aligned, false if not
 */
bool is_page_aligned(uint32_t *addr)
{
    return ((uint32_t)addr) % PAGE_SIZE == 0;
}

/**
 * Rounds addr down to the nearest page boundary
 */
uint32_t *get_page_aligned(uint32_t *addr)
{
    uint32_t iaddr = (uint32_t)addr;

    return (uint32_t *)(PAGE_SIZE * (iaddr / PAGE_SIZE));
}

/*
 * Kernel Functions
 */

/* Generic */

/**
 * Initialise paging for the kernel
 */
void k_init_paging(uint32_t kernel_physical_end)
{
    framebitmap = (uint32_t *)(kernel_physical_end + PHYS_OFFSET);

    uint32_t p_count = kernel_physical_end / PAGE_SIZE + 2;

    for (uint32_t i = 0; i < p_count; i++) {
        uint32_t pde_offset = i / 1024;
        uint32_t pte_offset = i % 1024;

        uint32_t p_addr = i * PAGE_SIZE;
        kernel_pte[pde_offset][pte_offset] =
            p_addr | PTE_PRESENT | PTE_RW | PTE_USER;

        if (!(kernel_pde[pde_offset + 768] & PTE_PRESENT)) {
            uint32_t p_pt = (uint32_t)kernel_pte[pde_offset] - PHYS_OFFSET;
            kernel_pde[pde_offset + 768] =
                p_pt | PTE_PRESENT | PTE_RW | PTE_USER;
        }

        last_page_addr = (PHYS_OFFSET) + (i * PAGE_SIZE);
    }

    load_paging_directory((uint32_t)kernel_pde - PHYS_OFFSET);

    for (uint32_t i = 0; i < p_count; i++) {
        k_mark_frame_occupied(i);
    }
}

/* Frames */

/**
 * Allocates a free frame, returns the physical address of the new frame
 */
uint32_t k_alloc_free_frame()
{
    for (uint32_t i = 0; i < TOTAL_BITMAP_NUMBER; i++) {
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

    return FRAME_ALLOC_FAIL;
}

/**
 * Marks a physical frame as occupied
 */
void k_mark_frame_occupied(uint32_t frame_no)
{
    uint32_t line_no = frame_no / 32;
    uint32_t offset = frame_no % 32;

    framebitmap[line_no] |= (0x1 << offset);
}

/**
 * Marks a physical frame as free
 */
void k_free_frame(uint32_t frame_no)
{
    uint32_t line_no = frame_no / 32;
    uint32_t offset = frame_no % 32;

    framebitmap[line_no] &= ~(0x1 << offset);
}

bool k_is_frame_occupied(uint32_t frame_no)
{
    uint32_t line_no = frame_no / 32;
    uint32_t offset = frame_no % 32;

    return framebitmap[line_no] & (0x1 << offset);
}

/* Pages */

/**
 * Maps a virtual address to a physical one in the pd/pt
 */
static void k_map_page(uint32_t vaddr, uint32_t paddr)
{
    KASSERT(is_page_aligned((uint32_t *)vaddr));
    KASSERT(is_page_aligned((uint32_t *)paddr));

    uint32_t pde_offset = (vaddr / PAGE_SIZE) / 1024;
    uint32_t pte_offset = (vaddr / PAGE_SIZE) % 1024;

    kernel_pte[pde_offset - 768][pte_offset] =
        paddr | PTE_PRESENT | PTE_RW | PTE_USER;

    if (!(kernel_pde[pde_offset] & PTE_PRESENT)) {
        uint32_t p_pt = (uint32_t)kernel_pte[pde_offset - 768] - PHYS_OFFSET;
        kernel_pde[pde_offset] = p_pt | PTE_PRESENT | PTE_RW | PTE_USER;
    }
}

/**
 * Returns the next available virtual address
 */
// static uint32_t get_next_vaddr() { return last_page_addr++; }

/**
 * Allocates a page and optionally maps it to a specific virtual address
 */
uint32_t *k_alloc_page(uint32_t *vaddr)
{
    if (vaddr != NULL)
        KASSERT(is_page_aligned(vaddr));
    else
        vaddr = (uint32_t *)(last_page_addr += PAGE_SIZE);

    uint32_t paddr = k_alloc_free_frame();
    k_map_page((uint32_t)vaddr, paddr);
    return vaddr;
}

/**
 * Frees the page and frame mapped to a specific virtual address
 */
void k_free_page(uint32_t *vaddr)
{
    KASSERT(is_page_aligned(vaddr));

    uint32_t i_paddr = (uint32_t)k_virt_to_phys(vaddr);
    uint32_t i_vaddr = (uint32_t)vaddr;

    uint32_t pde_offset = (i_vaddr / PAGE_SIZE) / 1024;
    uint32_t pte_offset = (i_vaddr / PAGE_SIZE) % 1024;
    uint32_t frame = i_paddr / PAGE_SIZE;

    kernel_pte[pde_offset - 768][pte_offset] = 0;
    k_free_frame(frame);
}

/**
 * Converts a virtual address to the mapped physical address
 */
uint32_t *k_virt_to_phys(uint32_t *vaddr)
{
    uint32_t i_vaddr = (uint32_t)vaddr;
    uint32_t pde_offset = (i_vaddr / PAGE_SIZE) / 1024;
    uint32_t pte_offset = (i_vaddr / PAGE_SIZE) % 1024;

    // discard flags, return address only
    return (uint32_t *)(kernel_pte[pde_offset - 768][pte_offset] & (~0xFFF));
}

/**
 * Returns true if the virtual address has been allocated, false if not
 */
bool k_is_virt_allocated(uint32_t *vaddr)
{
    KASSERT(is_page_aligned(vaddr));

    uint32_t i_vaddr = (uint32_t)vaddr;

    uint32_t pde_offset = (i_vaddr / PAGE_SIZE) / 1024;
    uint32_t pte_offset = (i_vaddr / PAGE_SIZE) % 1024;

    if (kernel_pde[pde_offset] & PTE_PRESENT) {
        if (kernel_pte[pde_offset - 768][pte_offset] & PTE_PRESENT) {
            return true;
        }
    }

    return false;
}

/* Processes */

static void map_page(pde_t *pd, uint32_t vaddr, uint32_t paddr,
                     uint32_t *virtual_pde)
{
    uint32_t pde_offset = (vaddr / PAGE_SIZE) / 1024;
    uint32_t pte_offset = (vaddr / PAGE_SIZE) % 1024;

    pte_t *pt_virt = NULL;

    // Check to see if we've already allocated this page table
    if (!(pd[pde_offset] & PTE_PRESENT)) {
        // Alloc a new page table
        uint32_t pt_virt_addr = (uint32_t)k_alloc_page(NULL);
        uint32_t phys_addr = (uint32_t)k_virt_to_phys((uint32_t *)pt_virt_addr);
        pd[pde_offset] = phys_addr | PTE_PRESENT | PTE_RW | PTE_USER;
        virtual_pde[pde_offset] = pt_virt_addr;

        kernel_virtual_pde[pde_offset] = pt_virt_addr;

        pt_virt = (pte_t *)pt_virt_addr;

        for (uint32_t i = 0; i < PAGE_SIZE / sizeof(pte_t); i++) {
            pt_virt[i] = 0;
        }
    }
    else {
        pt_virt = (pte_t *)kernel_virtual_pde[pde_offset];
    }

    pt_virt[pte_offset] = paddr | PTE_PRESENT | PTE_RW | PTE_USER;
}

/**
 * Allocates a new mapped page to a specific virtual addr
 */
uint32_t *alloc_page(pde_t *pd, uint32_t *vaddr, uint32_t *last_proc_page_addr,
                     uint32_t *virtual_pde)
{
    if (vaddr == NULL)
        vaddr = (uint32_t *)(*last_proc_page_addr += PAGE_SIZE);
    KASSERT(is_page_aligned(vaddr));

    uint32_t paddr = k_alloc_free_frame();
    map_page(pd, (uint32_t)vaddr, paddr, virtual_pde);

    return vaddr;
}

/**
 * Frees a virtual address
 *
 * virtual_pde -> The mapping from virt addr -> virt addr of the associated PT.
 */
void free_page(uint32_t *vaddr, uint32_t *virtual_pde)
{
    KASSERT(is_page_aligned(vaddr));

    // We free the virtual address
    uint32_t ivaddr = (uint32_t)vaddr;
    uint32_t pde_offset = (ivaddr / PAGE_SIZE) / 1024;
    uint32_t pte_offset = (ivaddr / PAGE_SIZE) % 1024;

    // Lookup the pt in the virtual pde -> that way we'll get the mapped virtual
    // address of the PT.
    pte_t *pt = (pte_t *)virtual_pde[pde_offset];
    pt[pte_offset] = 0;

    // TODO: free the physical memory at some point
}

/**
 * Translates a process virtual addr to phys
 */
uint32_t *virt_to_phys(uint32_t *virtual_pde, uint32_t *vaddr)
{
    // TODO: rewrite using process pd, pt properly - need to take into account
    // virt vs phys for the pt address
    uint32_t ivaddr = (uint32_t)vaddr;
    uint32_t pde_offset = (ivaddr / PAGE_SIZE) / 1024;
    uint32_t pte_offset = (ivaddr / PAGE_SIZE) % 1024;

    pte_t *pt = (pte_t *)virtual_pde[pde_offset];
    return (uint32_t *)(pt[pte_offset] & (!0xFFF));
}

/**
 * Given a virtual address and a virtual pde, is the address alrready allocated
 */
bool is_virt_allocated(uint32_t *virtual_pde, uint32_t *vaddr)
{
    uint32_t ivaddr = (uint32_t)vaddr;
    uint32_t pde_offset = (ivaddr / PAGE_SIZE) / 1024;
    uint32_t pte_offset = (ivaddr / PAGE_SIZE) % 1024;

    if (virtual_pde[pde_offset] & PTE_PRESENT) {
        pte_t *pt = (pte_t *)virtual_pde[pde_offset];
        return pt[pte_offset] & PTE_PRESENT;
    }

    return false;
}

/**
 * Creates a new page directory with the physical kernel mapings copied.
 */
pde_t *create_page_directory()
{
    pde_t *pd = k_alloc_page(NULL);

    for (uint32_t i = 0; i < 768; i++) {
        pd[i] = 0;
    }

    for (uint32_t i = 768; i < 1024; i++) {
        pd[i] = kernel_pde[i];
    }

    return pd;
}

/**
 * Creates a new page directory with the virtual kernel mappings copied.
 */
pde_t *create_virtual_page_directory()
{
    pde_t *pd = k_alloc_page(NULL);

    for (uint32_t i = 0; i < 768; i++) {
        pd[i] = 0;
    }

    for (uint32_t i = 768; i < 1024; i++) {
        pd[i] = kernel_virtual_pde[i];
    }

    return pd;
}

/**
 * Copies a region of memory from somewhere to somewhere. Size is the size in
 * bytes.
 */
void copy_mem(uint32_t *from, uint32_t *to, uint32_t size)
{
    uint8_t *cto = (uint8_t *)to;
    uint8_t *cfrom = (uint8_t *)from;

    for (uint32_t i = 0; i < size; i++) {
        cto[i] = cfrom[i];
    }
}

/**
 * Allocates a range of virtual addresses mapped to specific physical addresses
 *
 * Size = size in bytes
 */
uint32_t *map_virt_range(pde_t *pd, pde_t *virt_pd, uint32_t *vaddr,
                         uint32_t *paddr, uint32_t size)
{
    KASSERT(is_page_aligned(paddr));
    KASSERT(is_page_aligned(vaddr));

    uint32_t ivaddr = (uint32_t)vaddr;
    uint32_t ipaddr = (uint32_t)paddr;
    uint32_t p_count = (size / PAGE_SIZE) + 1;

    for (uint32_t i = 0; i < p_count; i++) {
        map_page(pd, ivaddr + (PAGE_SIZE * i), ipaddr + (PAGE_SIZE * i),
                 virt_pd);
    }

    return vaddr;
}
