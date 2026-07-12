#include "syscall.h"
#include "../drivers/vga.h"
#include "../idt.h"
#include "../misc/utils.h"
#include "process.h"
#include "../abi/stat.h"

void init_syscalls()
{
    register_interrupt_handler(INT_SYSCALL, handle_syscall);
}

void handle_syscall(struct cpu_state *cpu, struct stack_state *stack)
{
    UNUSED(stack);

    print("Got a syscall: ");
    printui(cpu->eax);
    print("\n");

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
        cpu->eax = (uint32_t)_brk;
        break;
    case SYS_FSTAT:
        struct abi_stat *st = (struct abi_stat*)cpu->ecx;
        st->st_mode = ABI_S_IFCHR;
        cpu->eax = 0;
        break;
    default:
        for (;;) ;
        break;
    }

    print("Finished handling syscall\n");
}
