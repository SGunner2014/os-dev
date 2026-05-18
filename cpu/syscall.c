#include "syscall.h"

void handle_syscall(struct cpu_state *cpu)
{
    switch(cpu->eax) {
        case SYS_WRITE:
            break;
    }
}
