#include "syscall.h"
#include "../drivers/vga.h"
#include "../idt.h"
#include "../misc/utils.h"
#include "process.h"

void init_syscalls()
{
    register_interrupt_handler(INT_SYSCALL, handle_syscall);
}

void handle_syscall(struct cpu_state *cpu, struct stack_state *stack)
{
    UNUSED(stack);

    print("Got a syscall\n");

    switch (cpu->eax) {
    case SYS_EXIT: // TODO: Does nothing atm, I should implement this
        for (;;)
            ;
        break;
    case SYS_WRITE:
        write((char *)cpu->ecx, cpu->edx);
        break;
    case SYS_BRK:
        print("Got a syscall for brk\n");
        Process *process = get_current_process();
        uint32_t *_brk = brk(process, (uint32_t *)cpu->ebx);
        asm volatile("mov %0, %%eax" : : "r"(_brk));
        break;
    default:
        break;
    }

    print("Finished handling syscall\n");
}
