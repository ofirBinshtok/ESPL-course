global _start
global main

extern printf
extern puts
extern strlen

section .rodata
    str: db "argc= %d" , 10, 0 

section .text
_start:
    pop    dword ecx    ; ecx = argc
    mov    esi,esp      ; esi = argv
    ;; lea eax, [esi+4*ecx+4] ; eax = envp = (4*ecx)+esi+4
    mov     eax,ecx     ; put the number of arguments into eax
    shl     eax,2       ; compute the size of argv in bytes
    add     eax,esi     ; add the size to the address of argv 
    add     eax,4       ; skip NULL at the end of argv
    push    dword eax   ; char *envp[]
    push    dword esi   ; char* argv[]
    push    dword ecx   ; int argc

    call    main        ; int main( int argc, char *argv[], char *envp[] )

    mov     ebx,eax
    mov     eax,1
    int     0x80
    nop
    
main:
    push ebp                ; save old stack
    mov ebp, esp
    pushad
    push dword str
    mov edi, [ebp+8]        ; gets argc
    call printf
    add esp, 4
    mov esi, [ebp+12]       ; gets argv 
    call next
    popad
    pop ebp
    mov eax, 0
    ret

; loop over argv
next:
    push edx                        ; because in print we override edx
    push dword[esi + 4 * edx]       ; argv[i]
    call strlen
    push eax
    call puts
    mov ecx, dword[esp+4]
    add esp, 8
    pop edx
    inc edx
    cmp edx, edi
    jnz next
    ret