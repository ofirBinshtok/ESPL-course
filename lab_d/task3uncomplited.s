global main
extern printf, malloc, fgets, stdin, strlen, free

section .data
    STATE: dw 0xACE1 ; An example initial state
    MASK:  dw 0x002D ; Fibonacci LFSR mask for 16 bits
    
section .bss
    buffer: resb 600
    struct_pointer: resd 1
section .rodata
newline: db 0xA,0
format: db "%02hhx",0
; the whole struct is in size 7
x_struct: db 5 ; double = 4 bytes
x_num: db 0xaa,1, 2, 0x44,0x4f
y_struct: db 6
y_num: db 0xaa, 1, 2, 3, 0x44, 0x4f
res_struct: db 0 ; the result struct
res_num: times 256 db 0 ; preallocate space for the result


section .text

main:
    push ebp
    mov ebp, esp
    pushad
    mov eax, [ebp+8] ;argc
    mov ebx, [ebp+12] ;argv
    cmp eax, 1
    jne more_than_one_argument
    ;call getmulti
    push y_struct
    push x_struct
    call add_multi
    add esp, 8
    push eax
    call print_multi
    call free
    add esp, 4
end_of_program:
    popad
    pop ebp
    ret
more_than_one_argument:
    mov ebx, dword[ebx + 4]         ;ebx = argv[1]
    cmp word[ebx], "-I"
    je I_option
    cmp word[ebx], "-R"
    je R_option
    jmp end_of_program
I_option:
    call getmulti
    push eax
    call getmulti
    push eax
    call add_multi
    push eax
    call print_multi
    call free
    add esp, 4
    call free
    add esp, 4
    call free
    add esp, 4
    jmp end_of_program
R_option:
    call PRmulti
    push eax
    call PRmulti
    push eax
    call add_multi
    push eax
    call print_multi
    call free
    add esp, 4
    call free
    add esp, 4
    call free
    add esp, 4
    jmp end_of_program
getMaxMin:
    movzx ecx, byte [eax] ; getting size of first struct
    movzx edx, byte [ebx] ; getting size of second struct
    cmp ecx, edx
    jae end ; if ecx >= edx, no need to swap

    ; swap eax and ebx
    xchg eax, ebx

end:
    ret ; return to caller


print_multi:
    push ebp
    mov ebp, esp
    pushad

    mov eax, [ebp+8] ; getting the pointer to the struct
    movzx edi, byte [eax] ; getting size - edi size is 4 bytes, and therefore only the size is saved
    cmp edi, 0
    jz finish

    lea esi, [eax+edi] ; calculate the address of the last byte



run_loop:
    movzx eax, byte [esi] ; load the byte into eax and zero-extend to 32 bits
    push eax
    push dword format
    call printf
    add esp, 8


    dec esi ; decrease the pointer to print in reverse order
    dec edi
    cmp edi, 0
    jnz run_loop ; if edi != 0, repeat


finish:
    push newline
    call printf
    add esp,4

    popad
    mov eax, 0
    leave
    ret

getmulti:   ;struct_multi* getmulti();
    push ebp
    mov ebp, esp
    pushad

    ;fgets(buffer, 600, stdin)
    push dword[stdin]
    push 600
    push buffer
    call fgets
    add esp, 12

    push buffer
    call strlen
    add esp, 4

    mov esi, eax
    sub esi, 2                     ;-1 for new line, and -1 for index
    shr eax, 1
    inc eax

    push eax
    call malloc
    mov dword[struct_pointer], eax
    mov edi, eax
    pop ebx
    dec ebx
    mov byte[edi], bl
    mov ebx, 1
    mov ecx, 0
loop_buffer:
    mov cx, word[buffer + esi - 1]
    cmp cl, '9'
    jg parse_letter
    sub cl, '0'
    jmp skip_parse_letter
parse_letter:
    sub cl, 'a'
    add cl, 0xa
skip_parse_letter:
    cmp ch, '9'
    jg parse_letter2
    sub ch, '0'
    jmp skip_parse_letter2
parse_letter2:
    sub ch, 'a'
    add ch, 0xa
skip_parse_letter2:
    ;cl = 1, ch = 2
    ;0000 xxxx          
    ;yyyy 0000
    ;yyyy xxxx
    shl cl, 4
    or cl, ch
    mov byte[edi + ebx], cl
    inc ebx
    sub esi, 2
    cmp esi, 0
    jge loop_buffer
    popad
    pop ebp
    mov eax, dword[struct_pointer]
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

    pop edi
    pop esi
    movzx eax, byte[esi]
    add eax, 2
    push eax
    call malloc
    pop ebx
    dec ebx
    mov dword[struct_pointer], eax
    mov byte[eax], bl
    mov ebx, 0
    mov ecx, 0
    mov edx, 0      ;count
loop_add_numbers:
    movzx ebx, byte[esi + 1 + edx]
    add ebx, ecx
    movzx ecx, byte[edi + 1 + edx]
    add ebx, ecx
    mov cl, bh
    mov byte[eax + 1 + edx], bl
    inc edx
    cmp dl, byte[edi]
    jne loop_add_numbers
    cmp dl, byte[esi]
    je skip_print
loop_add_numbers_2:
    movzx ebx, byte[esi + 1 + edx]
    add ebx, ecx
    mov cl, bh
    mov byte[eax + 1 + edx], bl
    inc edx
    cmp dl, byte[esi]
    jne loop_add_numbers_2
skip_print:
    mov byte[eax + 1 + edx], cl
    popad
    pop ebp
    mov eax, dword[struct_pointer]
    ret

rand_num:
    push ebp
    mov ebp, esp
    pushad
    mov ax, [STATE]
    and ax, [MASK]
    mov bx, 0
    parity_loop:
        mov cx, ax
        and cx, 1
        xor bx, cx
        shr ax, 1
        jnz parity_loop
    shr word [STATE], 1 ; Shift with parity as MSB
    shl bx, 15
    or word[STATE], bx
    popad
    pop ebp
    movzx eax, word[STATE]
    ret

PRmulti:
    push ebp
    mov ebp, esp
    pushad
    loop_till_not_zero:
    call rand_num ; Generate length
    cmp al, 0
    jz loop_till_not_zero ; If length is zero, try again
    movzx ecx, al ; ECX is counter for the number of bytes to generate
    add ecx, 1
    push ecx
    call malloc
    mov dword[struct_pointer], eax
    mov esi, eax
    pop ecx
    dec ecx
    mov byte[eax], cl
    mov ebx, 1
generate_loop:
    call rand_num
    mov byte[esi + ebx], al
    inc ebx
    loop generate_loop
    popad
    pop ebp
    mov eax, dword[struct_pointer]
    ret