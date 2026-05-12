global jump_user

; enters user mode
; stack: [esp + 12] -> eflags (control flags)
;        [esp +  8] -> esp
;        [esp +  4] -> eip (instruction pointer)
;        [esp +  0] -> return addr
jump_user:
    ; set up stack as iret expects
    mov ebx, [esp + 4]  ; eip
    mov ecx, [esp + 8]  ; esp
    mov edx, [esp + 12] ; eflags

    mov ax, (4 * 8) | 3
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax ; ss hanbdled by iret


    push (4 * 8) | 3    ; data selector
    push ecx ; esp
    push edx ; eflags
    push (3 * 8) | 3
    push ebx
    iret
