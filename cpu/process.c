#include "process.h"
#include "mem.h"
#include "malloc.h"
#include "../misc/utils.h"

static uint32_t last_pid = 0;
static Process *current_process = NULL;

Process *create_process()
{
    Process *process = kmalloc(sizeof(Process));
    process->pid = last_pid++;
    process->last_virt_addr = 0;


    process->heap.freep = NULL;

    uint32_t *page_directory = create_page_directory();
    process->page_directory_virt = page_directory;
    process->page_directory_phys = (pde_t*)((uint32_t)page_directory - PHYS_OFFSET);

    return process;
}

Process *get_current_process()
{
    return current_process;
}

void switch_process(Process *process)
{
    current_process = process;
    load_paging_directory((uint32_t) process->page_directory_phys);
}
