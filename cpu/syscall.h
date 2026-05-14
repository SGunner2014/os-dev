#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

#include "../idt.h"

void handle_syscall(struct cpu_state *cpu);

#endif
