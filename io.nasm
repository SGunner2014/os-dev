global outb
global inb

; outb - send a byte to an io port
; stack: [esp + 8] -> data byte
;        [esp + 4] -> io port
;        [esp + 0] -> return address
outb:
    mov al, [esp + 8]         ; move data to be sent into al register
    mov dx, [esp + 4]         ; move address of io port into dx register
    out dx, al                ; send data to io port
    ret                       ; return to calling func

; inb - read a byte from an io port
; stack: [esp + 4] -> port address
;        [esp + 0] -> return address
inb:
    mov dx, [esp + 4]
    in al, dx
    ret
