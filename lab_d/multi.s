global print_multi
global main
extern printf
extern fgets
extern stdin
extern strlen
extern malloc
extern free

BUFFER_SIZE EQU 600

section .bss
    buffer: resb BUFFER_SIZE            ; char buffer[BUFFER_SIZE]
    struct_helper: resd 1

section .data
    counter: dd 0
    x_struct: db 5 
    x_num:  db 0xaa,1,2,0x44,0x4f
    y_struct: db 5
    y_num: db 0xaa,1,2,3,0x44,0x4f

    STATE: dw 0xACE1
    MASK:  dw 0x002D

    for_Parity_use: dw 0x8000

section .rodata
    format: db "%02hhx", 0 
    format_newline: db 10,0

section .text
main:
    push ebp
    mov ebp, esp
    pushad
    
    mov eax, [ebp+8]            ; gets argc
    cmp eax,1                   ; means there's no arguments
    je default_func
    
    mov eax, [ebp+12]           ; gets argv
    mov eax, [eax+4]            ; gets argv[1]

    cmp word[eax], "-R"         ; comparing argv[1] with "-R"
    je PRNG_argument

    cmp word[eax], "-I"         ; comparing argv[1] with "-l"
    je stdin_argument

    popad
    pop ebp
    ret

    default_func: 
        push y_struct
        push x_struct
        call add_multi
        add esp, 8 
        push dword eax
        call print_multi
        add esp, 4

        popad
        mov esp, ebp
        pop ebp
        mov eax, 0
        ret
    
    stdin_argument:
        call get_multi              
        push eax                    ; pushing first num to stack
        call get_multi    
        push eax                    ; pushing second num to stack
        call add_multi
        push eax                    ; pushing the result of add_multi to the stack
        call print_multi
        add esp, 12
        popad 
        mov esp, ebp
        pop ebp
        ret
    
    PRNG_argument:
        call PRmulti
        push eax
        call PRmulti
        push eax
        call add_multi
        push eax
        call print_multi
        add esp, 12
        popad
        pop ebp
        ret

rand_num:
    push ebp
    mov ebp, esp
    pushad
    mov cx, 0

    ; even number of 1's = flag on, odd number of 1's = flag off
    compute_parity_flag:                
        mov ax, word[STATE]
        and ax, word[MASK]                      ; the result is in the parity flag

        jp flag_is1
        jnp flag_is0

        flag_is0:
            shr word[STATE], 1
            jmp contine

        flag_is1:
            shr word[STATE], 1
            mov ax, word[STATE]
            or ax, word[for_Parity_use]
            mov word[STATE], ax

        contine:
            inc cx
            cmp cx, 16
            jne compute_parity_flag
            popad
            pop ebp
            movzx eax, word[STATE]              ; movzx = padding of 0
            ret

PRmulti:
    push ebp
    mov ebp, esp
    pushad

    repeat:
        call rand_num                           ; Generate array length
        cmp al, 0
        je repeat
        movzx ebx, al
        inc ebx
        push ebx
        call malloc
        mov dword[struct_helper], eax
        pop ebx
        dec ebx
        mov byte[eax], bl
        mov esi, eax
        mov ecx, 0

    loop:
        call rand_num                           ; Generate random numbers to put in the struct
        mov byte[esi + ecx + 1], al
        dec ebx
        inc ecx
        cmp ebx, 0
        jne loop
        popad
        pop ebp
        mov eax, dword[struct_helper]
        ret

add_multi:
    push ebp
    mov ebp, esp
    pushad

    mov eax, [ebp+8]
    mov ebx, [ebp+12]
    call getMaxMin
    push eax
    call print_multi
    push ebx
    call print_multi
    add esp, 8

    mov ecx, 0
    mov cl, byte[eax]
    add ecx, 2
    pushad
    push ecx
    call malloc
    add esp, 4
    mov dword[struct_helper], eax
    popad
    mov edi, dword[struct_helper]
    dec ecx
    mov byte[edi], cl
    mov dword[counter], 0
    mov ecx, 0

ebx_looper:
    shr ecx, 8
    mov edx, 0
    mov esi, dword[counter]
    mov dl, byte[eax + esi + 1]
    add ecx, edx
    mov dl, byte[ebx + esi + 1]
    add ecx, edx
    mov byte[edi + esi + 1], cl
    inc dword[counter]
    mov dl, byte[counter]
    cmp dl, byte[ebx]
    jne ebx_looper
    cmp dl, byte[eax]
    je end_add

eax_looper:
    shr ecx, 8
    mov edx, 0
    mov esi, dword[counter]
    mov dl, byte[eax + esi + 1]
    add ecx, edx
    mov byte[edi + esi + 1], cl
    inc dword[counter]
    mov dl, byte[counter]
    cmp dl, byte[eax]
    jne eax_looper
    

end_add:
    shr ecx, 8
    mov byte[edi + esi + 2], cl
    popad
    mov esp, ebp
    pop ebp
    mov eax, dword[struct_helper]
    ret

getMaxMin:
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

    push dword [stdin]
    push dword BUFFER_SIZE          ; number of bytes to read
    push buffer                     ; buffer to read into
    call fgets
    add esp, 12

    push buffer
    call strlen
    add esp, 4
    mov esi, eax                    ; esi = strlen(buffer)
    shr eax, 1
    mov ebx, eax                    ; num size without size variable
    add eax, 1

    push eax
    call malloc
    add esp, 4
    mov dword[struct_helper], eax
    mov byte[eax], bl
    mov ebx, 0

input_looper:
    mov ecx, 0
    mov cl, byte[buffer + esi - 2]
    dec esi
    mov ch, byte[buffer + esi - 2]
    dec esi
    call ecx_parser
    mov byte[eax + 1 + ebx], ch
    inc ebx
    cmp esi, 0
    jle end_loop
    jmp input_looper

end_loop:
    popad
    mov esp, ebp
    mov eax, dword[struct_helper]
    pop ebp
    ret

ecx_parser:
    cmp cl, 'a'
    jl parseFirstNumber
    sub cl, 'a'
    add cl, 0xa

check_ch:
    cmp ch, 'a'
    jl parseSecondNumber
    sub ch, 'a'
    add ch, 0xa

continue_parse:
    shl cl, 4
    shl cx, 4
    ret

parseFirstNumber:
    sub cl, '0'
    jmp check_ch

parseSecondNumber:
    sub ch, '0'
    jmp continue_parse

print_multi:    
    push ebp
    mov ebp, esp
    pushad
    mov dword edx, 0
    mov dword eax, [ebp+8]      ; points to the beggining of the struct which contains the size of struct
    mov dword ecx, eax          ; ecx = adress of the size of the struct
    add ecx, 1                  ; ecx points to x_num 
    mov ebx, 0
    mov byte bl, [eax]          ; ebx = size of the struct
    call loop_over
    popad
    mov esp, ebp
    pop ebp
    ret

loop_over:
    mov edx, 0
    mov byte dl, [ecx+ebx-1]        ; printing the bytes from the end to the start
    pushad
    push edx                        ; num[i]     
    push dword format               ; push format to stack
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