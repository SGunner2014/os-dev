#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include "../types.h"
#include "mem.h"
#include "malloc.h"

struct process {
    uint32_t pid;
    uint32_t last_virt_addr;
    uint32_t *page_directory_phys;
    uint32_t *page_directory_virt;
    Heap heap;
};

typedef struct process Process;

Process *create_process();
Process *get_current_process();
void switch_process(Process *process);

#endif
