[BITS 32]
extern kernel_main
global _start
_start:
    mov esp, 0x90000
    cld
    call kernel_main
.halt:
    hlt
    jmp .halt
