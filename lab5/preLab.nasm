section .text
global _start

_start:
    ; Move the 64-bit value 0xb5 into the rcx register
    mov rcx, 0xb5

    ; Assume rbp is already set up as a base pointer for the stack frame
    ; and that local variables x, y, z are at -4, -8, -12 bytes from rbp

    ; Load the lower 8 bits of the variable y into lower 8 bits of the rax register
    movzx eax, byte [rbp-8]

    ; Load the address of the variable z into the rbx register
    lea rbx, [rbp-12]

    ; Store the 32-bit value in rax into the memory location whose address is in rbx
    mov [rbx], eax

    ; Exit the program (normally you would have system calls to handle program exit)
    mov rax, 60       ; syscall number for exit
    xor rdi, rdi      ; exit code 0
    syscall           ; invoke operating system to exit
