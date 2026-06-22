#ifndef INCLUDE_SYSCALL_H
#define INCLUDE_SYSCALL_H

#include "../idt.h"

#define SYS_EXIT        1
#define SYS_FORK        2
#define SYS_READ        3
#define SYS_WRITE       4
#define SYS_OPEN        5
#define SYS_CLOSE       6
#define SYS_LINK        9
#define SYS_UNLINK      10
#define SYS_EXECVE      11
#define SYS_STAT        18
#define SYS_LSEEK       19
#define SYS_GETPID      20
#define SYS_KILL        37
#define SYS_TIMES       43
#define SYS_BRK         45
#define SYS_FSTAT       108

void init_syscalls();
void handle_syscall(struct cpu_state *cpu, struct stack_state* stack);

#endif
