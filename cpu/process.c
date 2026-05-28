#include "process.h"
#include "mem.h"
#include "malloc.h"
#include "gdt.h"

static uint32_t last_pid = 0;
static Process *current_process = NULL;

static uint32_t* allocate_stack(pde_t *pd)
{
    uint32_t *addr = (uint32_t*) (PHYS_OFFSET - PAGE_SIZE);
    return allocate_virt_addr(pd, addr);
}

/**
 * Creates a new process with no memory setup, apart from page directory with
 * kernel mappings. Useful for loading ELF executables, etc.
 */
Process *create_empty_process()
{
    Process *process = kmalloc(sizeof(Process));

    process->pid = last_pid++;
    process->last_virt_addr = 0xFFFFFFF;

    process->heap.freep = NULL;
    uint32_t *page_directory = create_page_directory();
    process->page_directory_virt = page_directory;
    process->page_directory_phys = (pde_t*) ((uint32_t) page_directory - PHYS_OFFSET);

    process->stack = allocate_stack(process->page_directory_virt);

    process->kernel_stack = kmalloc(PAGE_SIZE);

    copy_kernel_mappings(process);

    return process;
}

/**
 * Creates a new process from a flat binary
 */
Process *create_process(uint32_t addr, uint32_t size)
{
    Process *process = kmalloc(sizeof(Process));

    process->pid = last_pid++;
    process->last_virt_addr = 0;
    process->exec_size = size;
    process->prog = kmalloc(size);

    copy_mem((uint32_t*) addr, (uint32_t*) process->prog, size);


    process->heap.freep = NULL;

    uint32_t *page_directory = create_page_directory();
    process->page_directory_virt = page_directory;
    process->page_directory_phys = (pde_t*)((uint32_t)page_directory - PHYS_OFFSET);

    // process->stack = kmalloc(PAGE_SIZE);
    process->stack = allocate_stack(process->page_directory_virt);
    process->kernel_stack = kmalloc(PAGE_SIZE);

    copy_kernel_mappings(process);

    return process;
}

Process *get_current_process()
{
    return current_process;
}

void switch_context(Process *process)
{
    current_process = process;
    load_paging_directory((uint32_t) process->page_directory_phys);
}

static inline uint32_t *stack_frame_to_stack_pointer(uint32_t *stack)
{
    return (uint32_t*) (((uint8_t*)stack) + PAGE_SIZE) - 4;
}

void exec_process(Process *process)
{
    uint32_t eflags = CPU_IF_FLAG;

    // Set the tss entry quickly so we know which process we're dealing
    // with in the future
    set_tss_stack_pointer(stack_frame_to_stack_pointer(process->kernel_stack));

    // Jump to user space
    jump_user(
        (uint32_t) process->prog,
        (uint32_t) stack_frame_to_stack_pointer(process->stack),
        eflags
    );

    for (;;) ;
}

/**
 * Maps a custom virtual range to a physical range
 * where size = size in bytes
 */
void map_custom_virt_range(
    Process *process, uint32_t *phys_start, uint32_t *virt_start,
    uint32_t size)
{
    pde_t *pd = process->page_directory_virt;

    // Now we need to figure out the page-aligned values for the start and end of the ranges
    uint32_t phys_aligned = (uint32_t) phys_start - ((uint32_t) phys_start % PAGE_SIZE);
    uint32_t virt_aligned = (uint32_t) virt_start - ((uint32_t) virt_start % PAGE_SIZE);
    uint32_t size_cnt = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    uint32_t size_algined = size_cnt * PAGE_SIZE;

    // Now we can make a call and map the pages
    map_virt_range_to_phys(
        pd,
        (uint32_t*) virt_aligned,
        (uint32_t*) phys_aligned,
        size_algined
    );
}
