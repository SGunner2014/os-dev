global loader

extern kmain

MAGIC_NUMBER equ 0x1BADB002
FLAGS equ 0x0
CHECKSUM equ -MAGIC_NUMBER
KERNEL_STACK_SIZE equ 4096    ; Size of stack in bytes


section .text
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM


loader:
    mov eax, 0xCAFEBABE
    mov esp, kernel_stack + KERNEL_STACK_SIZE

    call kmain
.loop:
    jmp .loop




section .bss
align 4                       ; Align at 4 bytes
kernel_stack:                 ; label points to memory beginning
    resb KERNEL_STACK_SIZE    ; reserve stack for the kernel
