;defines:

 SYS_WRITE EQU 4
 STDOUT EQU 1
 SYS_OPEN EQU 5

global _start
extern strlen
section .data
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
    push ebp                ; save old stack
    mov ebp, esp
    pushad
    mov edi, [ebp+8]        ; gets argc
    mov esi, [ebp+12]       ; gets argv  
    call next
    call encode
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
    call print
    mov ecx, dword[esp+4]
    add esp, 8
    call inputFile_outputFile
    pop edx
    inc edx
    cmp edx, edi
    jnz next
    ret

; prints arguments to stdout
print:
    mov eax, 4          ; write
    mov ebx, 1          ; stdout
    mov ecx, [esp+8]    ; putting the string we want to print into ecx
    mov edx, [esp+4]    ; count of bytes to print
    int 0x80
    cmp eax, 0
    jl exit_fail
    mov eax, 4          ; write
    mov ebx, 1          ; stdout
    mov ecx, newline    ; putting the string we want to print into ecx- a newline
    mov edx, 1          ; print 1 byte
    int 0x80
    cmp eax, 0
    jl exit_fail
    ret

inputFile_outputFile:
    cmp word[ecx], "-o"
    je open_output_file
    cmp word[ecx], "-i"
    je open_input_file
    ret

open_output_file:
    mov eax, 5          ; open file
    mov ebx, ecx
    add ebx, 2          ; step over "-o" or "-i" 
    mov ecx, 1101o      ; 1 = O_WRONLY, 100o = O_CREAT, 1000o = O_TRUNC
    mov edx, 644o       ; rw- r-- r--
    int 0x80
    cmp eax, 0
    jl exit_fail
    mov dword[outFile], eax
    ret

open_input_file:
    mov eax, 5          ; open file
    mov ebx, ecx
    add ebx, 2
    mov ecx, 0          ; O_RDONLY
    int 0x80
    cmp eax, 0
    jl exit_fail
    mov dword[inFile], eax
    ret

encode:

    read_char:
        mov eax, 3                      ; read
        mov ebx, dword[inFile]          
        mov ecx, char                   ; putting the chae we want to print into ecx
        mov edx, 1                      ; 1 byte to print
        int 0x80
        cmp eax, 0
        je finish_encoding
        jl exit_fail

    encode_char:    
        cmp byte[char], 'A'
        jl print_char
        cmp byte[char], 'z'
        jg print_char
        inc byte[char]                   ; increment by 1

    print_char:
        mov eax, 4                       ; write
        mov ebx, dword[outFile]          
        mov ecx, char                     ; the encoded char to print
        mov edx, 1
        int 0x80
        cmp eax, 0
        jl exit_fail
        jmp encode

    finish_encoding:
        ret

; exits the program when there is an error
exit_fail:
    mov ebx, 1     ; error code
    mov eax, 1     ; system exit
    int 0x80