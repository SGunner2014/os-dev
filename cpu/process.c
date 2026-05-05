#include "process.h"
#include "mem.h"
#include "malloc.h"
#include "../screen.h"
#include "../misc/utils.h"

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
    process->page_directory_phys = (pde_t*)((uint32_t)page_directory - PHYS_OFFSET);

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
    char buff[256];
    itoa((uint32_t) process->page_directory_phys, buff, 16);

    prints("Loading process pd @ ");
    prints(buff);
    prints("\n");

    load_paging_directory((uint32_t) process->page_directory_phys);
    prints("Loaded pd\n");
}
