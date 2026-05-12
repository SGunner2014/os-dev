global load_gdt
global load_ltr

; loads gdt table
; stack: [esp + 4] -> gdt address
;        [esp + 0] -> return address
load_gdt:
    mov eax, [esp + 4]
    lgdt [eax]

    ; Set data segment registers
    mov ax, 0x10
    mov ds, ax
    mov ss, ax
    mov es, ax

    ; Set code segment register - perform a far jump
    jmp 0x08:flush_cs

; loads tss
; stack: [esp + 4] -> tss selector
;        [esp + 0] -> return address
load_ltr:
    mov eax, [esp + 4]
    ltr ax
    ret

flush_cs:
    ret
