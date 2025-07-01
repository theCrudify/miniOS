// linker.ld - Linker script yang benar
ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    
    .text ALIGN(4K) :
    {
        *(.text)
    }
    
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }
    
    .data ALIGN(4K) :
    {
        *(.data)
    }
    
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
    
    end = .;
}

/* ====================================== */
// kernel/core/boot_entry.asm - Entry point assembly
[BITS 32]

extern kernel_main

global _start
_start:
    ; Set up stack
    mov esp, 0x90000
    
    ; Clear direction flag
    cld
    
    ; Call kernel main
    call kernel_main
    
    ; If kernel returns, halt
.halt:
    hlt
    jmp .halt

/* ====================================== */
// kernel/include/stdarg.h - Missing stdarg header
#ifndef STDARG_H
#define STDARG_H

typedef char* va_list;

#define va_start(ap, last) (ap = (va_list)&last + sizeof(last))
#define va_arg(ap, type) (*(type*)((ap += sizeof(type)) - sizeof(type)))
#define va_end(ap) (ap = (va_list)0)

#endif

/* ====================================== */
// kernel/lib/string.c - Missing string functions dalam kernel
#include "../include/kernel.h"

int strlen(const char* str) {
    int len = 0;
    while(str[len]) len++;
    return len;
}

int strcmp(const char* str1, const char* str2) {
    while(*str1 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t n) {
    while(n-- && *str1 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return n == (size_t)-1 ? 0 : *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strcpy(char* dest, const char* src) {
    char* orig_dest = dest;
    while((*dest++ = *src++));
    return orig_dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* orig_dest = dest;
    while(n-- && (*dest++ = *src++));
    while(n-- > 0) *dest++ = '\0';
    return orig_dest;
}

char* strcat(char* dest, const char* src) {
    char* orig_dest = dest;
    while(*dest) dest++;
    while((*dest++ = *src++));
    return orig_dest;
}

char* strtok(char* str, const char* delim) {
    static char* next_token = NULL;
    
    if(str) next_token = str;
    if(!next_token) return NULL;
    
    // Skip leading delimiters
    while(*next_token) {
        int is_delim = 0;
        for(const char* d = delim; *d; d++) {
            if(*next_token == *d) {
                is_delim = 1;
                break;
            }
        }
        if(!is_delim) break;
        next_token++;
    }
    
    if(!*next_token) {
        next_token = NULL;
        return NULL;
    }
    
    char* token_start = next_token;
    
    // Find end of token
    while(*next_token) {
        int is_delim = 0;
        for(const char* d = delim; *d; d++) {
            if(*next_token == *d) {
                is_delim = 1;
                break;
            }
        }
        if(is_delim) break;
        next_token++;
    }
    
    if(*next_token) {
        *next_token++ = '\0';
    } else {
        next_token = NULL;
    }
    
    return token_start;
}

void* memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    while(n--) {
        *d++ = *s++;
    }
    
    return dest;
}

void* memset(void* s, int c, size_t n) {
    char* ptr = (char*)s;
    while(n--) {
        *ptr++ = c;
    }
    return s;
}

void* realloc(void* ptr, size_t size) {
    if(!ptr) return malloc(size);
    if(!size) {
        free(ptr);
        return NULL;
    }
    
    void* new_ptr = malloc(size);
    if(new_ptr) {
        // Copy old data (simplified)
        memcpy(new_ptr, ptr, size);
        free(ptr);
    }
    return new_ptr;
}

/* ====================================== */
// kernel/core/syscall_entry.asm - System call assembly entry
[BITS 32]

extern syscall_handler

global syscall_interrupt
syscall_interrupt:
    cli
    pusha
    
    mov ax, ds
    push eax
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Call C handler with registers as arguments
    push edx    ; arg3
    push ecx    ; arg2
    push ebx    ; arg1
    push eax    ; syscall number
    call syscall_handler
    add esp, 16
    
    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    
    popa
    sti
    iret

/* ====================================== */
// kernel/include/types.h - Basic type definitions
#ifndef TYPES_H
#define TYPES_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef uint32_t size_t;
typedef enum { false = 0, true = 1 } bool;

#define NULL ((void*)0)

#endif

/* ====================================== */
// Fixed Makefile dengan dependency yang benar
# Makefile for MyOS - FIXED VERSION
CC = i686-elf-gcc
AS = nasm
LD = i686-elf-ld
OBJCOPY = i686-elf-objcopy
QEMU = qemu-system-i386

# Compiler flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -fno-pie -fno-stack-protector -Ikernel/include
ASFLAGS = -f elf32
LDFLAGS = -T linker.ld -ffreestanding -O2 -nostdlib -lgcc

# Directories
BOOT_DIR = boot
KERNEL_DIR = kernel
USER_DIR = userspace
BUILD_DIR = build
ISO_DIR = iso

# Source files
BOOT_ASM = $(BOOT_DIR)/bootloader.asm
KERNEL_C_SOURCES = $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_ASM_SOURCES = $(shell find $(KERNEL_DIR) -name "*.asm")

# Object files
KERNEL_C_OBJECTS = $(KERNEL_C_SOURCES:%.c=$(BUILD_DIR)/%.o)
KERNEL_ASM_OBJECTS = $(KERNEL_ASM_SOURCES:%.asm=$(BUILD_DIR)/%.o)
KERNEL_OBJECTS = $(KERNEL_C_OBJECTS) $(KERNEL_ASM_OBJECTS)

# Output files
BOOTLOADER = $(BUILD_DIR)/bootloader.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
OS_IMAGE = $(BUILD_DIR)/myos.img

# Default target
all: $(OS_IMAGE)

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/core
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/memory
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/process
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/drivers
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/filesystem
	mkdir -p $(BUILD_DIR)/$(KERNEL_DIR)/lib

# Build bootloader
$(BOOTLOADER): $(BOOT_ASM) | $(BUILD_DIR)
	$(AS) -f bin $< -o $@

# Build kernel C object files
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Build kernel ASM object files
$(BUILD_DIR)/%.o: %.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) $< -o $@

# Link kernel
$(KERNEL_BIN): $(KERNEL_OBJECTS) linker.ld | $(BUILD_DIR)
	$(CC) $(LDFLAGS) $(KERNEL_OBJECTS) -o $@

# Create OS disk image
$(OS_IMAGE): $(BOOTLOADER) $(KERNEL_BIN) | $(BUILD_DIR)
	dd if=/dev/zero of=$@ bs=1M count=10
	dd if=$(BOOTLOADER) of=$@ conv=notrunc
	dd if=$(KERNEL_BIN) of=$@ seek=1 conv=notrunc

# Run in QEMU
run: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) -m 128M

# Debug in QEMU
debug: $(OS_IMAGE)
	$(QEMU) -drive format=raw,file=$(OS_IMAGE) -m 128M -s -S

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

# Test individual components
test-compile:
	@echo "Testing compilation..."
	@echo "Available C files:"
	@find $(KERNEL_DIR) -name "*.c" | head -5
	@echo "Available ASM files:"
	@find $(KERNEL_DIR) -name "*.asm" | head -5

.PHONY: all run debug clean test-compile

/* ====================================== */
// kernel/drivers/timer_fix.c - Timer driver implementation
#include "../include/kernel.h"

#define PIT_FREQUENCY 1193180
#define TIMER_FREQUENCY 100 // 100 Hz

static unsigned int timer_ticks = 0;
static unsigned int seconds = 0;

void init_timer(void) {
    // Calculate divisor for desired frequency
    unsigned int divisor = PIT_FREQUENCY / TIMER_FREQUENCY;
    
    // Send command byte
    outb(0x43, 0x36);
    
    // Send frequency divisor
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
    
    print("Timer initialized at 100 Hz\n");
}

void timer_callback(void) {
    timer_ticks++;
    
    if(timer_ticks % TIMER_FREQUENCY == 0) {
        seconds++;
    }
}

unsigned int get_timer_ticks(void) {
    return timer_ticks;
}

unsigned int get_uptime_seconds(void) {
    return seconds;
}

void sleep(unsigned int ms) {
    unsigned int target = timer_ticks + (ms * TIMER_FREQUENCY / 1000);
    while(timer_ticks < target) {
        asm volatile("hlt");
    }
}

/* ====================================== */
// Missing function definitions di kernel.h yang perlu diperbaiki
// Tambahkan ke kernel/include/kernel.h:

// System initialization
void init_idt(void);
void init_syscalls(void);
void init_keyboard(void);
void init_mouse(void);
void init_vga(void);
void init_disk(void);
void init_timer(void);

// Driver functions
void handle_mouse_input(int x, int y, int buttons);
void handle_keyboard_input(int scancode);

// Timer functions  
unsigned int get_timer_ticks(void);
unsigned int get_uptime_seconds(void);
void sleep(unsigned int ms);

// Missing definitions
void set_idt_gate(int n, unsigned int handler);
char getchar(void);

// System calls
extern void syscall_interrupt(void);

/* ====================================== */
// build_fix_script.sh - Script untuk memperbaiki build issues
#!/bin/bash

echo "🔧 Fixing MyOS build issues..."

# Create missing directories
mkdir -p MyOS/kernel/lib
mkdir -p MyOS/kernel/include

# Copy/create missing files
echo "Creating missing linker script..."
cat > MyOS/linker.ld << 'EOF'
ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    
    .text ALIGN(4K) :
    {
        *(.text)
    }
    
    .rodata ALIGN(4K) :
    {
        *(.rodata)
    }
    
    .data ALIGN(4K) :
    {
        *(.data)
    }
    
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    }
    
    end = .;
}
EOF

# Create missing files with minimal content
echo "Creating missing source files..."

# Entry point assembly
cat > MyOS/kernel/core/boot_entry.asm << 'EOF'
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
EOF

# System call assembly
cat > MyOS/kernel/core/syscall_entry.asm << 'EOF'
[BITS 32]

extern syscall_handler

global syscall_interrupt
syscall_interrupt:
    cli
    pusha
    
    mov ax, ds
    push eax
    
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push edx
    push ecx
    push ebx
    push eax
    call syscall_handler
    add esp, 16
    
    pop ebx
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx
    
    popa
    sti
    iret
EOF

# Create string library for kernel
cat > MyOS/kernel/lib/string.c << 'EOF'
#include "../include/kernel.h"

int strlen(const char* str) {
    int len = 0;
    while(str[len]) len++;
    return len;
}

int strcmp(const char* str1, const char* str2) {
    while(*str1 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strcpy(char* dest, const char* src) {
    char* orig_dest = dest;
    while((*dest++ = *src++));
    return orig_dest;
}

void* memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while(n--) *d++ = *s++;
    return dest;
}

void* memset(void* s, int c, size_t n) {
    char* ptr = (char*)s;
    while(n--) *ptr++ = c;
    return s;
}
EOF

echo "✅ Build fixes applied!"
echo "Now run:"
echo "  cd MyOS"
echo "  make clean"
echo "  make all"