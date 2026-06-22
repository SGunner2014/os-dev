#ifndef INCLUDE_PROCESS_H
#define INCLUDE_PROCESS_H

#include "../types.h"
#include "malloc.h"

#define CPU_CARRY_FLAG (1u << 0)
#define CPU_IF_FLAG (1u << 9)

typedef uint32_t pde_t;

typedef void (*call_module_t)(void);
struct process {
    uint32_t pid;
    uint32_t last_virt_addr;
    uint32_t *page_directory_phys;
    uint32_t *stack;
    uint32_t *kernel_stack;
    uint32_t *start_brk;
    uint32_t *brk;
    pde_t *page_directory_virt;
    uint32_t *virtual_pde;
    Heap heap;
    uint32_t exec_size;
    call_module_t prog;
};

typedef struct process Process;

struct tss_entry {
    uint32_t prev_tss; // Location of previous tss
    uint32_t esp0;     // Stack pointer to load when changing to kernel mode
    uint32_t ss0;      // Stack segment to load when changing to kernel mode
                       // Everything below here is unused.
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

typedef struct tss_entry Tss_Entry;

void jump_user(uint32_t eip, uint32_t esp, uint32_t eflags);
Process *create_empty_process();
Process *create_process(uint32_t addr, uint32_t size);
Process *get_current_process();
void switch_context(Process *process);
void exec_process(Process *process);
void map_custom_virt_range(Process *process, uint32_t *phys_start,
                           uint32_t *virt_start, uint32_t size);

#endif
