
rand_num:
    push ebp
    mov epb, esp
    pushad
    mov cx, 0

    ;even number of 1's- flag on, odd number of 1's- flag off
    compute_parity_flag:                
        mov ax, word[STATE]
        and ax, word[MASK]      ;the result is in the parity flag

        jp flag_is1
        jnp flag_is0

        flag_is0:
        shr word[STATE], 1
        jmp contine

        flag_is1:
        shr word[STATE], 1
        or word[STATE], for_Parity_use

        contine:
        inc cx, 1
        cmp cx, 16
        jne compute_parity_flag
        popad
        pop ebp
        movzx eax, word[STATE]      ;padding of 0
        ret

PRmulti:
    push ebp
    mov ebp, esp
    pushad
    repeat:
        call rand_num                       ; Generate array length
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
        call rand_num                       ; Generate random numbers to put in the struct
        mov byte[esi + ecx + 1], al
        dec ebx
        cmp ebx, 0
        jne loop
        popad
        pop ebp
        mov eax, dword[struct_helper]
        ret

