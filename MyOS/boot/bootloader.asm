; boot/bootloader.asm
; Simple bootloader for MyOS

[BITS 16]
[ORG 0x7C00]

start:
    ; Initialize segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7C00
    
    ; Clear screen
    mov ah, 0x00
    mov al, 0x03
    int 0x10
    
    ; Print boot message
    mov si, boot_msg
    call print_string
    
    ; Load kernel from disk
    mov ah, 0x02        ; Read sectors
    mov al, 10          ; Number of sectors to read
    mov ch, 0           ; Cylinder
    mov dh, 0           ; Head
    mov cl, 2           ; Sector (start from sector 2)
    mov dl, 0x80        ; Drive number (first hard disk)
    mov bx, 0x1000      ; Buffer address
    int 0x13
    
    jc disk_error
    
    ; Switch to protected mode
    cli
    lgdt [gdt_descriptor]
    
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp 0x08:protected_mode
    
disk_error:
    mov si, disk_error_msg
    call print_string
    jmp $
    
print_string:
    mov ah, 0x0E
.loop:
    lodsb
    cmp al, 0
    je .done
    int 0x10
    jmp .loop
.done:
    ret

[BITS 32]
protected_mode:
    ; Set up protected mode segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000
    
    ; Jump to kernel
    jmp 0x1000
    
; Global Descriptor Table
gdt_start:
    ; Null descriptor
    dd 0x0, 0x0
    
    ; Code segment
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10011010b    ; Access byte
    db 11001111b    ; Flags, Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)
    
    ; Data segment
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10010010b    ; Access byte
    db 11001111b    ; Flags, Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

boot_msg db 'MyOS Bootloader v1.0', 0x0D, 0x0A, 'Loading kernel...', 0x0D, 0x0A, 0
disk_error_msg db 'Disk read error!', 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55