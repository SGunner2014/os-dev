#include "process.h"
#include "mem.h"
#include "malloc.h"
#include "../misc/utils.h"
#include "../screen.h"

static uint32_t last_pid = 0;
static Process *current_process = NULL;

Process *create_process(uint32_t addr, uint32_t size)
{
    Process *process = kmalloc(sizeof(Process));
    process->pid = last_pid++;
    process->last_virt_addr = 0;
    process->exec_size = size;
    process->prog = kmalloc(size);

    prints("Allocated space\n");

    char buff[64];
    itoa((uint32_t) addr, buff, 16);
    prints("Prog: ");
    prints(buff);
    prints("\n");

    copy_mem((uint32_t*) addr, (uint32_t*) process->prog, size);

    prints("Copied mem");


    process->heap.freep = NULL;

    uint32_t *page_directory = create_page_directory();
    process->page_directory_virt = page_directory;
    process->page_directory_phys = (pde_t*)((uint32_t)page_directory - PHYS_OFFSET);
    process->stack = kmalloc(PAGE_SIZE);

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

void exec_process(Process *process)
{
    UNUSED(process);
}
