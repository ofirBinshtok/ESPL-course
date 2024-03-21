global print_multi
global main
extern printf
extern fgets
extern stdin
extern strlen
extern malloc
BUFFER_SIZE EQU 600
section .bss
    buffer: resb BUFFER_SIZE    ;char buffer[BUFFER_SIZE];
    struct_helper: resd 1
section .data
    x_struct: db 5 
    x_num: db 0xaa, 1,2,0x44,0x4f 
    y_struct: db 6 
    y_num: db 0xaa, 1,2,3,0x44,0x4f 

section .rodata
    format: db "%02hhx", 0 
    format_newline: db 10,0

section .text
main:
    push ebp
    mov ebp, esp
    pushad
    call get_multi
    
    push dword eax
    call print_multi
    add esp, 4
    popad
    mov esp, ebp
    pop ebp
    mov eax, 0
    ret

get_maxMin:
    ;eax = struct
    ;ebx = struct
    mov cl, byte[eax]
    cmp cl, byte[ebx]
    jge no_swap
    mov ecx, eax
    mov eax, ebx
    mov ebx, ecx
    no_swap:
    ret

get_multi:
    push ebp
    mov ebp, esp
    pushad

    push    dword [stdin]
    push    dword BUFFER_SIZE ; number of bytes to read
    push    buffer            ; buffer to read into
    call    fgets
    add     esp, 12

    push    buffer
    call    strlen
    add     esp, 4
    mov     esi, eax            ;esi = strlen(buffer)
    shr     eax, 1
    mov     ebx, eax           ;num size without size variable
    add     eax, 1

    push    eax
    call    malloc
    add     esp, 4
    mov     dword[struct_helper], eax
    mov     byte[eax], bl
    mov     ebx, 0
loop_over_input:
    mov     ecx, 0
    mov     cl, byte[buffer + esi - 2]
    dec     esi
    mov     ch, byte[buffer + esi - 2]
    dec     esi
    call    parse_ecx
    mov     byte[eax + 1 + ebx], ch
    inc     ebx
    cmp     esi, 0
    jle     finish_loop
    jmp     loop_over_input
    ;2 01 23        123\n   4/2 + 1
    ;2 12 34        1234\n  5/2 + 1
finish_loop:
    popad
    mov esp, ebp
    mov eax, dword[struct_helper]
    pop ebp
    ret
parse_ecx:
    cmp     cl, 'a'
    jl      parse_number1
    sub     cl, 'a'
    add     cl, 0xa
check_ch:
    cmp     ch, 'a'
    jl      parse_number2
    sub     ch, 'a'
    add     ch, 0xa
continue_parse:
    ;chcl = 0000 yyyy 0000 xxxx
    ;chcl = 0000 yyyy xxxx 0000 
    ;chcl = yyyy xxxx 0000 0000
    shl cl, 4
    shl cx, 4
    ;ch  = yyyy xxxx
    ret

parse_number1:
    sub     cl, '0'
    jmp     check_ch

parse_number2:
    sub     ch, '0'
    jmp     continue_parse
print_multi:    
    push ebp
    mov ebp, esp
    pushad
    mov dword edx, 0
    mov dword eax, [ebp+8]   ; points to the beggining of the struct which contains the size of struct
    mov dword ecx, eax       ; ecx = adress of the size of the struct
    add ecx, 1               ; ecx points to x_num 
    mov ebx, 0
    mov byte bl, [eax]     ; ebx = size of the struct
    call loop_over
    popad
    mov esp, ebp
    pop ebp
    ret

loop_over:
    mov edx, 0
    mov byte dl, [ecx+ebx-1]   ; printing the bytes from the end to the start
    pushad
    push edx                    ; num[i]     
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