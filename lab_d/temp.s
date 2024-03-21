


global main
extern printf, malloc, fgets, stdin, strlen, free
section .data
    STATE: dw 0xACE1 ; An example initial state
    MASK:  dw 0x002D ; Fibonacci LFSR mask for 16 bits
    
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
