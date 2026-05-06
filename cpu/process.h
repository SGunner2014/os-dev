#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include "../types.h"
#include "malloc.h"
#include "mem.h"

typedef void (*call_module_t)(void);
struct process {
    uint32_t pid;
    uint32_t last_virt_addr;
    uint32_t *page_directory_phys;
    uint32_t *stack;
    pde_t *page_directory_virt;
    Heap heap;
    uint32_t exec_size;
    call_module_t prog;
};

typedef struct process Process;


Process *create_process(uint32_t addr, uint32_t size);
Process *get_current_process();
void switch_context(Process *process);
void exec_process(Process *process);

#endif
