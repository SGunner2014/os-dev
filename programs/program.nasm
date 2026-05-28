[bits 32]

section .text

mov eax, dword 4
mov ebx, testString
mov ecx, 12
int 0x80

jmp $

section .data

testString: db 'Hello, world', 0
