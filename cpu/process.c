#include "process.h"
#include "mem.h"
#include "malloc.h"
#include "gdt.h"
#include "../misc/utils.h"
#include "../drivers/vga.h"

static uint32_t last_pid = 0;
static Process *current_process = NULL;

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

    process->stack = kmalloc(PAGE_SIZE);
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

static uint32_t *stack_frame_to_stack_pointer(uint32_t *stack)
{
    return (uint32_t*) (((uint8_t*)stack) + PAGE_SIZE) - 1;
}

void exec_process(Process *process)
{
    uint32_t eflags = CPU_IF_FLAG;

    // Set the tss entry quickly so we know which process we're dealing
    // with in the future
    set_tss_stack_pointer(stack_frame_to_stack_pointer(process->kernel_stack));

    print("jumping to user\n");

    char buff[64];
    itoa((uint32_t) process->prog, buff, 16);
    print(buff);
    print("\n");

    // Jump to user space
    jump_user(
        (uint32_t) process->prog,
        (uint32_t) stack_frame_to_stack_pointer(process->stack),
        eflags
    );

    for (;;) ;
}
