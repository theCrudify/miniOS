; kernel/core/interrupt_asm.asm
; Assembly interrupt handlers

[BITS 32]

; External C functions
extern exception_handler
extern irq_handler

; ISR macro for exceptions without error code
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push byte 0      ; Push dummy error code
    push byte %1     ; Push interrupt number
    jmp isr_common_stub
%endmacro

; ISR macro for exceptions with error code
%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push byte %1     ; Push interrupt number
    jmp isr_common_stub
%endmacro

; IRQ macro
%macro IRQ 2
global irq%1
irq%1:
    cli
    push byte 0      ; Push dummy error code
    push byte %2     ; Push IRQ number
    jmp irq_common_stub
%endmacro

; Define ISRs for exceptions (0-31)
ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

; Define IRQs (32-47)
IRQ 0, 32    ; Timer
IRQ 1, 33    ; Keyboard
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44   ; Mouse
IRQ 13, 45
IRQ 14, 46   ; Primary ATA
IRQ 15, 47   ; Secondary ATA

; Common ISR stub
isr_common_stub:
    pusha                    ; Push all general purpose registers
    
    mov ax, ds               ; Lower 16-bits of eax = ds
    push eax                 ; Save data segment descriptor
    
    mov ax, 0x10             ; Load kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call exception_handler   ; Call C exception handler
    
    pop eax                  ; Reload original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                     ; Pop all general purpose registers
    add esp, 8               ; Clean up pushed error code and ISR number
    sti
    iret                     ; Return from interrupt

; Common IRQ stub
irq_common_stub:
    pusha                    ; Push all general purpose registers
    
    mov ax, ds               ; Lower 16-bits of eax = ds
    push eax                 ; Save data segment descriptor
    
    mov ax, 0x10             ; Load kernel data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    call irq_handler         ; Call C IRQ handler
    
    pop eax                  ; Reload original data segment descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    popa                     ; Pop all general purpose registers
    add esp, 8               ; Clean up pushed error code and IRQ number
    sti
    iret                     ; Return from interrupt