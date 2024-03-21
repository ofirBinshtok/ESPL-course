global print_multi
global main
extern printf

section .data
    x_struct: db 5
    x_num: db 0xaa, 1, 2, 0x44, 0x4f

section .rodata
    format: db "%02hhx" , 0 
    format_newline: db 10,0

section .text
main:
    push ebp
    mov ebp, esp
    pushad
    push dword x_struct
    call print_multi
    add esp, 4
    popad
    mov esp, ebp
    pop ebp
    mov eax, 0
    ret

print_multi:    
    push ebp
    mov ebp, esp
    pushad
    mov dword edx, 0
    mov dword eax, [ebp+8]   ; points to the beggining of the struct which contains the size of struct
    mov dword ecx, eax       ; ecx = adress of the size of the struct
    add ecx, 1               ; ecx points to x_num 
    mov ebx, 0
    mov byte bl, [eax]       ; ebx = size of the struct
    call loop_over
    popad
    mov esp, ebp
    pop ebp
    ret

loop_over:
    mov edx, 0
    mov byte dl, [ecx+ebx-1]   ; printing the bytes from the end to the start
    pushad
    push edx                   ; num[i]     
    push dword format          ; push format to stack
    call printf
    add esp, 8
    popad
    dec ebx
    jnz loop_over
    pushad
    push format_newline
    call printf
    add esp, 4
    popad
    ret