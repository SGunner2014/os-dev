global jump_user

; enters user mode
; stack: [esp + 12] -> eflags (control flags)
;        [esp +  8] -> esp
;        [esp +  4] -> eip (instruction pointer)
;        [esp +  0] -> return addr
jump_user:
    mov ax, (4 * 8) | 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; set up stack as iret expects
    mov eax, [esp + 4]  ; eip
    mov ebx, [esp + 8]  ; ss
    mov ecx, [esp + 12] ; eflags
    push (4 * 8) | 3    ; data selector
    push ebx
    push ecx
    push (3 * 8) | 3
    push eax
    iret
