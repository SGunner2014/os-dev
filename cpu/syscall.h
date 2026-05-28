#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

#include "../idt.h"

#define SYS_EXIT        1
#define SYS_FORK        2
#define SYS_READ        3
#define SYS_WRITE       4

void init_syscalls();
void handle_syscall(struct cpu_state *cpu, struct stack_state* stack);

#endif
