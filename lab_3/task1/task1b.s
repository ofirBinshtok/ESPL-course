;defines:

 SYS_WRITE EQU 4
 STDOUT EQU 1

global _start
extern strlen
section .data
    data_ptr: dd 0
    newline: db 10
    char: db 0
    outFile: dd 1
    inFile: dd 0
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
    push ebp
    mov ebp, esp
    pushad
  ;  push ebx
    mov edi, [ebp+8]        ; gets argc
    mov esi, [ebp+12]       ; gets argv
    
    call next
    call encoder
    popad
    pop ebp
    mov eax, 0
    ret
; loop over argv
next:
    push edx
    push dword[esi + 4 * edx]    ; putting the string we want to print into ecx    (argv+1)[0]
    call strlen
    push eax
    call print
    add esp, 8
    pop edx
    inc edx
    cmp edx, edi
    jnz next
    ret
; prints arguments to stdout
print:
    mov eax, 4          ; write
    mov ebx, 1          ; stdout
    mov ecx, [esp+8]    ; putting the string we want to print into ecx    (argv+1)[0]
    mov edx, [esp+4]        ; count of bytes to print
    int 0x80
    cmp eax, 0
    jl exit_fail
    mov eax, 4          ; write
    mov ebx, 1          ; stdout
    mov ecx, newline    ; putting the string we want to print into ecx    (argv+1)[0]
    mov edx, 1        ; count of bytes to print
    int 0x80
    cmp eax, 0
    jl exit_fail
    ret

encoder:
    read_char:
        mov eax, 3          ; read
        mov ebx, dword[inFile]          ; stdout
        mov ecx, char    ; putting the string we want to print into ecx    (argv+1)[0]
        mov edx, 1        ; count of bytes to print
        int 0x80
        cmp eax, 0
        je finish_encoder
        jl exit_fail
    encode_char:    
        cmp byte[char], 'A'
        jl print_char
        cmp byte[char], 'z'
        jg print_char
        inc byte[char]
    print_char:
        mov eax, 4          ; write
        mov ebx, dword[outFile]          ; stdout
        mov ecx, char    ; putting the string we want to print into ecx    (argv+1)[0]
        mov edx, 1        ; count of bytes to print
        int 0x80
        cmp eax, 0
        jl exit_fail
        jmp encoder
    finish_encoder:
        ret

; exits the program "normally" with status 0
exit_fail:
    mov ebx, 1     ; error code
    mov eax, 1     ; system exit
    int 0x80
