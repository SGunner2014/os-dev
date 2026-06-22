#include "process.h"
#include "../drivers/vga.h"
#include "../misc/utils.h"
#include "../types.h"
#include "gdt.h"
#include "malloc.h"
#include "mem.h"

static uint32_t last_pid = 0;
static Process *current_process = NULL;

static uint32_t *allocate_stack(Process *process)
{
    uint32_t *addr = (uint32_t *)(PHYS_OFFSET - PAGE_SIZE);
    return alloc_page(process->page_directory_virt, addr,
                      &process->last_virt_addr, process->virtual_pde);
}

/**
 * Creates a new process with no memory setup, apart from page directory with
 * kernel mappings. Useful for loading ELF executables, etc.
 */
Process *create_empty_process()
{
    print("Mallocing\n");
    Process *process = kmalloc(sizeof(Process));
    print("Malloc'd\n");

    process->pid = last_pid++;
    process->last_virt_addr = 0xFFFFFFF;

    process->heap.freep = NULL;
    print("Creating pd\n");
    uint32_t *page_directory = create_page_directory();
    print("Created pd\n");
    process->page_directory_virt = page_directory;
    process->page_directory_phys =
        (pde_t *)((uint32_t)page_directory - PHYS_OFFSET);
    process->virtual_pde = create_virtual_page_directory();

    print("Allocating stack\n");

    process->stack = allocate_stack(process);

    print("Allocated stack\n");

    process->kernel_stack = kmalloc(PAGE_SIZE);

    return process;
}

/**
 * Creates a new process from a flat binary
 */
Process *create_process(uint32_t addr, uint32_t size)
{
    // Process *process = kmalloc(sizeof(Process));
    Process *process = create_empty_process();

    process->exec_size = size;
    process->prog = kmalloc(size);

    copy_mem((uint32_t *)addr, (uint32_t *)process->prog, size);

    return process;
}

Process *get_current_process() { return current_process; }

void switch_context(Process *process)
{
    current_process = process;
    load_paging_directory((uint32_t)process->page_directory_phys);
}

static inline uint32_t *stack_frame_to_stack_pointer(uint32_t *stack)
{
    return (uint32_t *)(((uint8_t *)stack) + PAGE_SIZE) - 4;
}

void exec_process(Process *process)
{
    uint32_t eflags = CPU_IF_FLAG;

    print("setting tss\n");

    // Set the tss entry quickly so we know which process we're dealing
    // with in the future
    set_tss_stack_pointer(stack_frame_to_stack_pointer(process->kernel_stack));

    print("Set tss\n");

    // Jump to user space
    jump_user((uint32_t)process->prog,
              (uint32_t)stack_frame_to_stack_pointer(process->stack), eflags);

    for (;;)
        ;
}

/**
 * Maps a custom virtual range to a physical range
 * where size = size in bytes
 */
void map_custom_virt_range(Process *process, uint32_t *phys_start,
                           uint32_t *virt_start, uint32_t size)
{
    // Now we need to figure out the page-aligned values for the start and end
    // of the ranges
    uint32_t phys_aligned =
        (uint32_t)phys_start - ((uint32_t)phys_start % PAGE_SIZE);
    uint32_t virt_aligned =
        (uint32_t)virt_start - ((uint32_t)virt_start % PAGE_SIZE);
    uint32_t size_cnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t size_algined = size_cnt * PAGE_SIZE;

    // Now we can make a call and map the pages
    // map_virt_range_to_phys(pd, (uint32_t *)virt_aligned,
    //                       (uint32_t *)phys_aligned, size_algined)
    map_virt_range(process->page_directory_virt, process->virtual_pde,
                   (uint32_t *)virt_aligned, (uint32_t *)phys_aligned,
                   size_algined);
}

uint32_t *brk(Process *process, uint32_t *new_brk)
{
    if (new_brk < process->start_brk) {
        return process->brk;
    }

    uint32_t *old_brk = process->brk;

    uint32_t aligned_old_brk = (uint32_t)get_page_aligned(old_brk);
    uint32_t aligned_new_brk = (uint32_t)get_page_aligned(new_brk);

    uint32_t old_index = aligned_old_brk / PAGE_SIZE;
    uint32_t page_diff = (aligned_new_brk - aligned_old_brk) / PAGE_SIZE;

    UNUSED(page_diff);
    UNUSED(old_index);

    return 0;
}
