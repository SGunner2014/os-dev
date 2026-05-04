#include "process.h"
#include "mem.h"
#include "malloc.h"
#include "../screen.h"

static uint32_t last_pid = 0;
static Process *current_process = NULL;

Process *create_process()
{
    prints("kmalloc'd");
    Process *process = kmalloc(sizeof(Process));
    process->pid = last_pid++;
    process->last_virt_addr = 0;


    process->heap.freep = NULL;

    uint32_t *page_directory = create_page_directory();
    process->page_directory_virt = page_directory;
    process->page_directory_phys = page_directory - PHYS_OFFSET;

    prints("returned from create process");

    return process;
}

Process *get_current_process()
{
    return current_process;
}

void switch_process(Process *process)
{
    current_process = process;
}
