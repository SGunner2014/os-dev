[bits 32]
[org 0xC0211F90]

section .text

mov eax, dword 4
mov ebx, testString
mov ecx, 13
int 0x80

mov eax, dword 4
mov ebx, testString
mov ecx, 12
int 0x80

jmp $

section .data

testString: db 'Hello, world', 0
