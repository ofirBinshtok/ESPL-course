global main
extern printf
extern puts

section .rodata
    argc: db "argc= %d" , 10, 0 

section .text

main: 
    push ebp                ; save old stack
    mov ebp, esp
    pushad
    push dword[ebp+8]       ; gets argc
    push dword argc
    call printf
    add esp, 8
    mov edx, [ebp+8]         ; argc
    mov eax, [ebp+12]       ; gets argv[0]  
    add edx, 0
    call next
    popad
    mov esp, ebp
    pop ebp
    mov eax, 0
    ret

; loop over argv
next:
    pushad
    push dword[eax]       ; argv[i]
    call puts
    add esp, 4
    popad
    add eax, 4
    dec edx
    jnz next
    ret
    

    

