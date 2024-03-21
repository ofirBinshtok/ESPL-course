section .data
    str: db "hello world", 0xA
    len equ $ - str
section .text
global _start

_start:    
    mov eax, 4
    mov ebx, 1
    mov ecx, str
    mov edx, len
    int 0x80

    ; exits the program with status 0
    mov ebx, 0     ; error code
    mov eax, 1     ; system exit
    int 0x80
