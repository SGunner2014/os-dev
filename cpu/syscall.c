#include "syscall.h"
#include "../drivers/vga.h"
#include "../idt.h"
#include "../misc/utils.h"

void init_syscalls()
{
    register_interrupt_handler(INT_SYSCALL, handle_syscall);
}

void handle_syscall(struct cpu_state *cpu, struct stack_state* stack)
{
    UNUSED(stack);

    switch(cpu->eax) {
        case SYS_EXIT: // TODO: Does nothing atm, I should implement this
            for (;;) ;
            break;
        case SYS_WRITE:
            write((char *) cpu->ebx, cpu->ecx);
            break;
    }
}
