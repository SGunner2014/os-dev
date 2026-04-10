global load_idt
global isr_stub_table

extern handle_interrupt

; loads interrupt descriptor table
; stack: [esp + 4] -> idt[0] address
;        [esp + 0] -> return address
load_idt:
    mov eax, [esp + 4]
    lidt [eax] ; load new idt
    sti ; set interrupt flag
    ret

%macro no_error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword 0    ; push 0 as error code
    push dword %1   ; push interrupt number
    jmp common_interrupt_handler
%endmacro

%macro error_code_interrupt_handler 1
global interrupt_handler_%1
interrupt_handler_%1:
    push dword %1
    jmp common_interrupt_handler
%endmacro

; Common interrupt handler:
common_interrupt_handler:
    ; save registers
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp

    push esp
    call handle_interrupt
    add esp, 4

    pop ebp
    pop edi
    pop esi
    pop edx
    pop ecx
    pop ebx
    pop eax

    ; restore esp (we skip the interrupt num and err code)
    add esp, 8

    ; return to code that was interrupted
    iret

; cpu interrupts
no_error_code_interrupt_handler 0
no_error_code_interrupt_handler 1
no_error_code_interrupt_handler 2
no_error_code_interrupt_handler 3
no_error_code_interrupt_handler 4
no_error_code_interrupt_handler 5
no_error_code_interrupt_handler 6
no_error_code_interrupt_handler 7
error_code_interrupt_handler 8
no_error_code_interrupt_handler 9
error_code_interrupt_handler 10
error_code_interrupt_handler 11
error_code_interrupt_handler 12
error_code_interrupt_handler 13
error_code_interrupt_handler 14
no_error_code_interrupt_handler 15
no_error_code_interrupt_handler 16
error_code_interrupt_handler 17
no_error_code_interrupt_handler 18
no_error_code_interrupt_handler 19
no_error_code_interrupt_handler 20
error_code_interrupt_handler 21
no_error_code_interrupt_handler 22
no_error_code_interrupt_handler 23
no_error_code_interrupt_handler 24
no_error_code_interrupt_handler 25
no_error_code_interrupt_handler 26
no_error_code_interrupt_handler 27
no_error_code_interrupt_handler 28
no_error_code_interrupt_handler 29
error_code_interrupt_handler 30
no_error_code_interrupt_handler 31

; irqs - pics
no_error_code_interrupt_handler 32   ; IRQ0  - timer
no_error_code_interrupt_handler 33   ; IRQ1  - keyboard
no_error_code_interrupt_handler 34   ; IRQ2  - cascade (slave PIC)
no_error_code_interrupt_handler 35   ; IRQ3  - COM2
no_error_code_interrupt_handler 36   ; IRQ4  - COM1
no_error_code_interrupt_handler 37   ; IRQ5  - LPT2
no_error_code_interrupt_handler 38   ; IRQ6  - floppy
no_error_code_interrupt_handler 39   ; IRQ7  - LPT1
no_error_code_interrupt_handler 40   ; IRQ8  - real time clock
no_error_code_interrupt_handler 41   ; IRQ9  - general I/O
no_error_code_interrupt_handler 42   ; IRQ10 - general I/O
no_error_code_interrupt_handler 43   ; IRQ11 - general I/O
no_error_code_interrupt_handler 44   ; IRQ12 - PS/2 mouse
no_error_code_interrupt_handler 45   ; IRQ13 - coprocessor
no_error_code_interrupt_handler 46   ; IRQ14 - IDE bus
no_error_code_interrupt_handler 47   ; IRQ15 - IDE bus

isr_stub_table:
    dd interrupt_handler_0
    dd interrupt_handler_1
    dd interrupt_handler_2
    dd interrupt_handler_3
    dd interrupt_handler_4
    dd interrupt_handler_5
    dd interrupt_handler_6
    dd interrupt_handler_7
    dd interrupt_handler_8
    dd interrupt_handler_9
    dd interrupt_handler_10
    dd interrupt_handler_11
    dd interrupt_handler_12
    dd interrupt_handler_13
    dd interrupt_handler_14
    dd interrupt_handler_15
    dd interrupt_handler_16
    dd interrupt_handler_17
    dd interrupt_handler_18
    dd interrupt_handler_19
    dd interrupt_handler_20
    dd interrupt_handler_21
    dd interrupt_handler_22
    dd interrupt_handler_23
    dd interrupt_handler_24
    dd interrupt_handler_25
    dd interrupt_handler_26
    dd interrupt_handler_27
    dd interrupt_handler_28
    dd interrupt_handler_29
    dd interrupt_handler_30
    dd interrupt_handler_31
    dd interrupt_handler_32
    dd interrupt_handler_33
    dd interrupt_handler_34
    dd interrupt_handler_35
    dd interrupt_handler_36
    dd interrupt_handler_37
    dd interrupt_handler_38
    dd interrupt_handler_39
    dd interrupt_handler_40
    dd interrupt_handler_41
    dd interrupt_handler_42
    dd interrupt_handler_43
    dd interrupt_handler_44
    dd interrupt_handler_45
    dd interrupt_handler_46
    dd interrupt_handler_47
