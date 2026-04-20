global loader

extern kmain

extern _kernel_start
extern _kernel_physical_start
extern _kernel_end
extern _kernel_physical_end

MAGIC_NUMBER equ 0x1BADB002
FLAGS equ 0x0
CHECKSUM equ -MAGIC_NUMBER
KERNEL_STACK_SIZE equ 4096    ; Size of stack in bytes

section .multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

section .text

loader:
    mov dx, 0x3F8
    mov al, 'A'
    out dx, al

    ; We need to set pde[0] and pde[768] to point both to boot_page_table
    ; First we will need to build the page table in question

    mov edi, (boot_page_table - 0xC0000000) ; Get the actual location of th boot page table
    mov esi, 0x0 ; We're starting mapping from 0x0
    mov ecx, 1024 ; We want to map 1024 pages, this will be the loop counter

.map_loop:
    mov edx, esi ; store the current address in edx
    or edx, 0x003 ; add the flags for present + read/write
    mov [edi], edx ; store the page record inside the page table
    add esi, 0x1000 ; move the address to the next 4kb page
    add edi, 4 ; The location of the next page table entry will be 4 bytes ahead.
    loop .map_loop ; Loop back to the marker IF COUNT != 0

    mov eax, (boot_page_table - 0xC0000000) ; move location of page table into eax -> will become pde entry
    or eax, 0x003 ; mark pde entry as present + read/write
    mov [(boot_page_directory - 0xC0000000) + 0 * 4], eax ; move the pde entry into the pde at entry #0
    mov [(boot_page_directory - 0xC0000000) + 768 * 4], eax ; move the pde entry into the pde at entry #768

    ; move location of pde into cr3
    mov ecx, (boot_page_directory - 0xC0000000)
    mov cr3, ecx

    ; load cr0 into ecx, set PG bit, move back to cr0
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

    ; PAGING IS NOW ENABLED
    lea ecx, [higher_half]
    jmp ecx

higher_half:
    ; at this stage we have now set eip -> 0xC0100xxx
    ; we can now remove the identity map from pde[0]
    mov dword [(boot_page_directory) + 0 * 4], 0
    invlpg [0] ; flush tlb entry for identity mapping

    mov esp, kernel_stack + KERNEL_STACK_SIZE

    push _kernel_start
    push _kernel_physical_start
    push _kernel_end
    push _kernel_physical_end

    call kmain

    cli
    hlt




section .bss
align 4                       ; Align at 4 bytes
kernel_stack:                 ; label points to memory beginning
    resb KERNEL_STACK_SIZE    ; reserve stack for the kernel

align 4096
boot_page_directory:
    resd 1024

boot_page_table:
    resd 1024
