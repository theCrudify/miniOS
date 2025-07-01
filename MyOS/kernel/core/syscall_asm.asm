// kernel/core/syscall_asm.asm
; Assembly system call handler

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

// kernel/core/boot_init.asm
; Additional boot initialization

[BITS 32]

global _start
extern kernel_main

section .text
_start:
    ; Set up stack
    mov esp, 0x90000
    
    ; Clear direction flag
    cld
    
    ; Call kernel main
    call kernel_main
    
    ; If kernel_main returns, halt
halt_loop:
    hlt
    jmp halt_loop

// userspace/utilities/ps.c
// Process list utility

#include "../lib/libc/stdio.h"
#include "../lib/libc/stdlib.h"

int main(void) {
    printf("  PID  PPID STATE     COMMAND\n");
    printf("    1     0 RUNNING   kernel\n");
    printf("    2     1 RUNNING   desktop\n");
    printf("    3     1 RUNNING   shell\n");
    printf("    4     1 SLEEPING  background_task\n");
    
    return 0;
}

// userspace/utilities/ls.c
// List files utility

#include "../lib/libc/stdio.h"
#include "../lib/libc/stdlib.h"
#include "../../kernel/include/filesystem.h"

int main(int argc, char** argv) {
    char* path = (argc > 1) ? argv[1] : ".";
    char** files = list_directory(path);
    
    if(!files) {
        printf("ls: cannot access '%s': No such file or directory\n", path);
        return 1;
    }
    
    for(int i = 0; files[i] != NULL; i++) {
        printf("%s  ", files[i]);
        free(files[i]);
    }
    printf("\n");
    free(files);
    
    return 0;
}

// userspace/utilities/cp.c
// Copy file utility

#include "../lib/libc/stdio.h"
#include "../lib/libc/stdlib.h"
#include "../../kernel/include/filesystem.h"

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("Usage: cp <source> <destination>\n");
        return 1;
    }
    
    if(copy_file(argv[1], argv[2]) != 0) {
        printf("cp: cannot copy '%s' to '%s'\n", argv[1], argv[2]);
        return 1;
    }
    
    return 0;
}

// userspace/utilities/mv.c
// Move file utility

#include "../lib/libc/stdio.h"
#include "../lib/libc/stdlib.h"
#include "../../kernel/include/filesystem.h"

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("Usage: mv <source> <destination>\n");
        return 1;
    }
    
    if(move_file(argv[1], argv[2]) != 0) {
        printf("mv: cannot move '%s' to '%s'\n", argv[1], argv[2]);
        return 1;
    }
    
    return 0;
}

// userspace/utilities/rm.c
// Remove file utility

#include "../lib/libc/stdio.h"
#include "../lib/libc/stdlib.h"
#include "../../kernel/include/filesystem.h"

int main(int argc, char** argv) {
    if(argc != 2) {
        printf("Usage: rm <filename>\n");
        return 1;
    }
    
    if(delete_file(argv[1]) != 0) {
        printf("rm: cannot remove '%s': No such file or directory\n", argv[1]);
        return 1;
    }
    
    return 0;
}

// linker.ld
/* Linker script for MyOS kernel */

ENTRY(_start)

SECTIONS
{
    . = 0x100000;
    
    .text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
    }
    
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - 0xC0000000)
    {
        *(.rodata)
    }
    
    .data ALIGN(4K) : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
    }
    
    .bss ALIGN(4K) : AT(ADDR(.bss) - 0xC0000000)
    {
        *(COMMON)
        *(.bss)
    }
    
    end = .;
}

// boot/grub.cfg
menuentry "MyOS" {
    set root=(hd0,1)
    multiboot /boot/kernel.bin
    boot
}

// config/system.conf
# MyOS System Configuration File

# Kernel Configuration
kernel_version=1.0
debug_mode=false
log_level=info

# Memory Configuration
memory_size=128MB
heap_size=16MB
stack_size=1MB

# Graphics Configuration
gui_enabled=true
screen_width=1024
screen_height=768
color_depth=32

# Network Configuration
network_enabled=true
dhcp_enabled=true
default_gateway=192.168.1.1
dns_server=8.8.8.8

# File System Configuration
root_fs=ext2
max_open_files=256
buffer_cache_size=4MB

# Process Configuration
max_processes=256
default_priority=10
time_slice=10ms

// config/network.conf
# Network Configuration

# Interface Settings
interface=eth0
mac_address=52:54:00:12:34:56

# IP Configuration
ip_address=192.168.1.100
netmask=255.255.255.0
gateway=192.168.1.1
broadcast=192.168.1.255

# DNS Configuration
primary_dns=8.8.8.8
secondary_dns=8.8.4.4

# DHCP Configuration
dhcp_enabled=true
dhcp_timeout=30

// config/users.conf
# User Configuration

# Root user
root:x:0:0:root:/root:/bin/shell

# System users
daemon:x:1:1:daemon:/usr/sbin:/bin/false
bin:x:2:2:bin:/bin:/bin/false
sys:x:3:3:sys:/dev:/bin/false

# Regular users
user:x:1000:1000:Default User:/home/user:/bin/shell
guest:x:1001:1001:Guest User:/home/guest:/bin/shell

// tools/qemu-debug.sh
#!/bin/bash

echo "Starting MyOS in QEMU with debugging..."
qemu-system-i386 \
    -drive format=raw,file=build/myos.img \
    -m 128M \
    -s -S \
    -monitor stdio \
    -serial file:debug.log

// userspace/gui/applications/file_manager/file_manager.c
// Simple file manager application

#include "../../../lib/libgui/window.h"
#include "../../../lib/libgui/graphics.h"
#include "../../../../kernel/include/filesystem.h"

typedef struct {
    Window* window;
    char current_path[256];
    char** file_list;
    int selected_index;
    int scroll_offset;
} FileManager;

static FileManager fm;

void init_file_manager(void) {
    fm.window = create_window("File Manager", 100, 100, 600, 400);
    strcpy(fm.current_path, "/");
    fm.selected_index = -1;
    fm.scroll_offset = 0;
    refresh_file_list();
}

void refresh_file_list(void) {
    if(fm.file_list) {
        // Free old list
        for(int i = 0; fm.file_list[i]; i++) {
            free(fm.file_list[i]);
        }
        free(fm.file_list);
    }
    
    fm.file_list = list_directory(fm.current_path);
}

void draw_file_manager(void) {
    if(!fm.window || !fm.window->visible) return;
    
    // Clear window
    gui_draw_rectangle(fm.window->x, fm.window->y + 25, 
                      fm.window->width, fm.window->height - 25, 
                      COLOR_WHITE);
    
    // Draw path bar
    gui_draw_rectangle(fm.window->x, fm.window->y + 25, 
                      fm.window->width, 30, COLOR_LIGHT_GRAY);
    gui_draw_text(fm.window->x + 5, fm.window->y + 35, 
                 fm.current_path, COLOR_BLACK);
    
    // Draw file list
    if(fm.file_list) {
        int y = fm.window->y + 60;
        for(int i = fm.scroll_offset; fm.file_list[i] && y < fm.window->y + fm.window->height - 20; i++) {
            unsigned int bg_color = (i == fm.selected_index) ? COLOR_BLUE : COLOR_WHITE;
            unsigned int text_color = (i == fm.selected_index) ? COLOR_WHITE : COLOR_BLACK;
            
            gui_draw_rectangle(fm.window->x + 5, y, fm.window->width - 10, 20, bg_color);
            gui_draw_text(fm.window->x + 10, y + 5, fm.file_list[i], text_color);
            
            y += 22;
        }
    }
}

void handle_file_manager_event(GUIEvent* event) {
    if(event->type == EVENT_MOUSE_CLICK) {
        if(event->x >= fm.window->x && event->x < fm.window->x + fm.window->width &&
           event->y >= fm.window->y + 60 && event->y < fm.window->y + fm.window->height) {
            
            int index = (event->y - fm.window->y - 60) / 22 + fm.scroll_offset;
            if(fm.file_list && fm.file_list[index]) {
                fm.selected_index = index;
                
                // Double click to open
                if(event->button == 1) {
                    // Navigate to directory or open file
                    char new_path[512];
                    snprintf(new_path, sizeof(new_path), "%s/%s", fm.current_path, fm.file_list[index]);
                    
                    // Check if it's a directory
                    struct vfs_node* node = find_file(new_path);
                    if(node && node->type == FILE_TYPE_DIRECTORY) {
                        strcpy(fm.current_path, new_path);
                        refresh_file_list();
                        fm.selected_index = -1;
                    }
                }
            }
        }
    }
}

int file_manager_main(void) {
    init_file_manager();
    
    GUIEvent event;
    while(1) {
        if(gui_get_event(&event)) {
            handle_file_manager_event(&event);
        }
        
        draw_file_manager();
        gui_flush();
        
        // Small delay
        for(volatile int i = 0; i < 100000; i++);
    }
    
    return 0;
}

// userspace/gui/applications/text_editor/text_editor.c
// Simple text editor

#include "../../../lib/libgui/window.h"
#include "../../../lib/libgui/graphics.h"

typedef struct {
    Window* window;
    char* text_buffer;
    int buffer_size;
    int cursor_pos;
    int scroll_x, scroll_y;
    char filename[256];
    int modified;
} TextEditor;

static TextEditor editor;

void init_text_editor(void) {
    editor.window = create_window("Text Editor", 150, 150, 500, 350);
    editor.buffer_size = 4096;
    editor.text_buffer = malloc(editor.buffer_size);
    editor.text_buffer[0] = '\0';
    editor.cursor_pos = 0;
    editor.scroll_x = 0;
    editor.scroll_y = 0;
    strcpy(editor.filename, "untitled.txt");
    editor.modified = 0;
}

void draw_text_editor(void) {
    if(!editor.window || !editor.window->visible) return;
    
    // Clear window
    gui_draw_rectangle(editor.window->x, editor.window->y + 25,
                      editor.window->width, editor.window->height - 25,
                      COLOR_WHITE);
    
    // Draw text content
    int x = editor.window->x + 5;
    int y = editor.window->y + 30;
    int line = 0;
    int col = 0;
    
    for(int i = 0; i < strlen(editor.text_buffer); i++) {
        char c = editor.text_buffer[i];
        
        if(c == '\n') {
            line++;
            col = 0;
            y += 16;
            x = editor.window->x + 5;
        } else {
            if(line >= editor.scroll_y && 
               y < editor.window->y + editor.window->height - 10) {
                char str[2] = {c, '\0'};
                gui_draw_text(x, y, str, COLOR_BLACK);
            }
            x += 8;
            col++;
        }
        
        // Draw cursor
        if(i == editor.cursor_pos) {
            gui_draw_rectangle(x, y, 2, 16, COLOR_BLACK);
        }
    }
    
    // Draw status bar
    gui_draw_rectangle(editor.window->x, 
                      editor.window->y + editor.window->height - 20,
                      editor.window->width, 20, COLOR_LIGHT_GRAY);
    
    char status[256];
    snprintf(status, sizeof(status), "%s%s | Line: %d", 
             editor.filename, editor.modified ? "*" : "", line + 1);
    gui_draw_text(editor.window->x + 5, 
                 editor.window->y + editor.window->height - 15,
                 status, COLOR_BLACK);
}

void handle_text_editor_event(GUIEvent* event) {
    if(event->type == EVENT_KEY_PRESS) {
        if(event->key >= 32 && event->key <= 126) {
            // Insert character
            if(editor.cursor_pos < editor.buffer_size - 1) {
                // Shift text right
                for(int i = strlen(editor.text_buffer); i >= editor.cursor_pos; i--) {
                    editor.text_buffer[i + 1] = editor.text_buffer[i];
                }
                editor.text_buffer[editor.cursor_pos] = event->key;
                editor.cursor_pos++;
                editor.modified = 1;
            }
        } else if(event->key == '\b') {
            // Backspace
            if(editor.cursor_pos > 0) {
                editor.cursor_pos--;
                // Shift text left
                for(int i = editor.cursor_pos; i < strlen(editor.text_buffer); i++) {
                    editor.text_buffer[i] = editor.text_buffer[i + 1];
                }
                editor.modified = 1;
            }
        } else if(event->key == '\n') {
            // Enter
            if(editor.cursor_pos < editor.buffer_size - 1) {
                // Insert newline
                for(int i = strlen(editor.text_buffer); i >= editor.cursor_pos; i--) {
                    editor.text_buffer[i + 1] = editor.text_buffer[i];
                }
                editor.text_buffer[editor.cursor_pos] = '\n';
                editor.cursor_pos++;
                editor.modified = 1;
            }
        }
    }
}

int text_editor_main(void) {
    init_text_editor();
    
    GUIEvent event;
    while(1) {
        if(gui_get_event(&event)) {
            handle_text_editor_event(&event);
        }
        
        draw_text_editor();
        gui_flush();
        
        // Small delay
        for(volatile int i = 0; i < 100000; i++);
    }
    
    return 0;
}

// userspace/gui/applications/calculator/calculator.c
// Simple calculator application

#include "../../../lib/libgui/window.h"
#include "../../../lib/libgui/graphics.h"
#include "../../../lib/libc/math.h"

typedef struct {
    Window* window;
    char display[32];
    double current_value;
    double stored_value;
    char operation;
    int new_number;
} Calculator;

static Calculator calc;

void init_calculator(void) {
    calc.window = create_window("Calculator", 400, 300, 200, 250);
    strcpy(calc.display, "0");
    calc.current_value = 0;
    calc.stored_value = 0;
    calc.operation = 0;
    calc.new_number = 1;
}

void draw_calculator(void) {
    if(!calc.window || !calc.window->visible) return;
    
    // Clear window
    gui_draw_rectangle(calc.window->x, calc.window->y + 25,
                      calc.window->width, calc.window->height - 25,
                      COLOR_LIGHT_GRAY);
    
    // Draw display
    gui_draw_rectangle(calc.window->x + 10, calc.window->y + 35,
                      calc.window->width - 20, 30, COLOR_WHITE);
    gui_draw_text(calc.window->x + 15, calc.window->y + 45,
                 calc.display, COLOR_BLACK);
    
    // Draw buttons
    char* buttons[] = {
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", ".", "=", "+"
    };
    
    int x = calc.window->x + 10;
    int y = calc.window->y + 75;
    
    for(int i = 0; i < 16; i++) {
        int col = i % 4;
        int row = i / 4;
        
        int btn_x = x + col * 45;
        int btn_y = y + row * 35;
        
        // Button background
        gui_draw_rectangle(btn_x, btn_y, 40, 30, COLOR_GRAY);
        
        // Button text
        gui_draw_text(btn_x + 15, btn_y + 10, buttons[i], COLOR_BLACK);
    }
}

void handle_calculator_event(GUIEvent* event) {
    if(event->type == EVENT_MOUSE_CLICK) {
        int btn_x = calc.window->x + 10;
        int btn_y = calc.window->y + 75;
        
        // Check if click is on button area
        if(event->x >= btn_x && event->x < btn_x + 180 &&
           event->y >= btn_y && event->y < btn_y + 140) {
            
            int col = (event->x - btn_x) / 45;
            int row = (event->y - btn_y) / 35;
            int button_index = row * 4 + col;
            
            if(button_index >= 0 && button_index < 16) {
                char* buttons[] = {
                    "7", "8", "9", "/",
                    "4", "5", "6", "*",
                    "1", "2", "3", "-",
                    "0", ".", "=", "+"
                };
                
                char pressed = buttons[button_index][0];
                
                if(pressed >= '0' && pressed <= '9') {
                    // Number input
                    if(calc.new_number) {
                        strcpy(calc.display, "");
                        calc.new_number = 0;
                    }
                    
                    if(strlen(calc.display) < 15) {
                        char digit[2] = {pressed, '\0'};
                        strcat(calc.display, digit);
                    }
                } else if(pressed == '.') {
                    // Decimal point
                    if(!strchr(calc.display, '.')) {
                        strcat(calc.display, ".");
                    }
                } else if(pressed == '=') {
                    // Calculate result
                    double display_val = atof(calc.display);
                    
                    switch(calc.operation) {
                        case '+':
                            calc.current_value = calc.stored_value + display_val;
                            break;
                        case '-':
                            calc.current_value = calc.stored_value - display_val;
                            break;
                        case '*':
                            calc.current_value = calc.stored_value * display_val;
                            break;
                        case '/':
                            if(display_val != 0) {
                                calc.current_value = calc.stored_value / display_val;
                            }
                            break;
                    }
                    
                    sprintf(calc.display, "%.6g", calc.current_value);
                    calc.operation = 0;
                    calc.new_number = 1;
                } else {
                    // Operation
                    calc.stored_value = atof(calc.display);
                    calc.operation = pressed;
                    calc.new_number = 1;
                }
            }
        }
    }
}

int calculator_main(void) {
    init_calculator();
    
    GUIEvent event;
    while(1) {
        if(gui_get_event(&event)) {
            handle_calculator_event(&event);
        }
        
        draw_calculator();
        gui_flush();
        
        // Small delay
        for(volatile int i = 0; i < 100000; i++);
    }
    
    return 0;
}