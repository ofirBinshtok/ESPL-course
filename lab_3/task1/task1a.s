;defines:

 SYS_WRITE EQU 4
 STDOUT EQU 1

global _start
extern strlen
section .data
    data_ptr: dd 0
    newline: db 10
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

; exits the program "normally" with status 0
exit_fail:
    mov ebx, 1     ; error code
    mov eax, 1     ; system exit
    int 0x80

; ; Define system call constants
; %define SYS_WRITE 4
; %define STDOUT 1
; %define SYS_EXIT 1

; ; Define global entry point
; global _start

; section .data
;     ; Declare a pointer for storing the current argument being printed
;     arg_ptr: dd 0

; section .text
; _start:
;     ; Set up stack frame
;     push ebp
;     mov ebp, esp

;     ; Save registers
;     push ebx
;     push edi
;     push esi

;     ; Get argc and argv from the stack
;     mov ebx, [ebp+8]   ; argc
;     mov edi, [ebp+12]  ; argv

;     ; Loop through each argument and print it to stdout
;     mov ecx, ebx       ; Counter for number of arguments
;     mov esi, edi       ; Pointer to the first argument
;     .loop_args:
;         ; Get pointer to current argument and store it in arg_ptr
;         mov eax, [esi]
;         mov [arg_ptr], eax

;         ; Call strlen to determine the length of the argument
;         push dword [arg_ptr]
;         call strlen
;         add esp, 4    ; Clean up the stack

;         ; Write argument to stdout
;         mov ebx, STDOUT
;         mov ecx, [arg_ptr]
;         mov edx, eax
;         mov eax, SYS_WRITE
;         int 0x80

;         ; Write newline to stdout
;         mov ebx, STDOUT
;         mov ecx, newline
;         mov edx, 1
;         mov eax, SYS_WRITE
;         int 0x80

;         ; Increment pointer to next argument and decrement counter
;         add esi, 4
;         dec ecx
;         jnz .loop_args

;     ; Clean up and exit program
;     pop esi
;     pop edi
;     pop ebx
;     mov esp, ebp
;     pop ebp
;     mov eax, 0        ; Set return value to 0
;     mov ebx, SYS_EXIT
;     int 0x80

; section .data
;     newline db 10, 0   ; Define newline character for output
