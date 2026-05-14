[bits 32]

mov [0xC00B8000], 'A'

int 0x80

jmp $
