// kernel/core/kernel.c
// Main kernel file for MyOS

#include "../include/kernel.h"
#include "../include/memory.h"
#include "../include/graphics.h"

// VGA text buffer
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// Colors
#define VGA_COLOR_BLACK 0
#define VGA_COLOR_BLUE 1
#define VGA_COLOR_GREEN 2
#define VGA_COLOR_RED 4
#define VGA_COLOR_WHITE 15

static int cursor_x = 0;
static int cursor_y = 0;
static char* vga_buffer = (char*)VGA_MEMORY;

// Function prototypes
void kernel_main(void);
void init_kernel(void);
void init_memory(void);
void init_interrupts(void);
void init_drivers(void);
void init_filesystem(void);
void init_gui(void);
void print(const char* str);
void print_colored(const char* str, int color);
void clear_screen(void);
void update_cursor(void);

// Kernel entry point
void kernel_main(void) {
    clear_screen();
    
    print_colored("MyOS Kernel v1.0", VGA_COLOR_GREEN);
    print("\nInitializing system components...\n");
    
    init_kernel();
    init_memory();
    init_interrupts();
    init_drivers();
    init_filesystem();
    init_gui();
    
    print_colored("\nSystem initialized successfully!", VGA_COLOR_GREEN);
    print("\nStarting GUI...\n");
    
    // Start the GUI
    start_desktop_environment();
    
    // Kernel main loop
    while(1) {
        // Handle system tasks
        schedule_processes();
        handle_interrupts();
        update_gui();
        
        // Small delay to prevent 100% CPU usage
        for(volatile int i = 0; i < 1000000; i++);
    }
}

void init_kernel(void) {
    print("Initializing kernel core... ");
    
    // Initialize kernel data structures
    // Set up system tables
    // Initialize critical sections
    
    print_colored("OK\n", VGA_COLOR_GREEN);
}

void init_memory(void) {
    print("Initializing memory management... ");
    
    // Initialize physical memory manager
    init_physical_memory();
    
    // Initialize virtual memory (paging)
    init_paging();
    
    // Initialize heap
    init_heap();
    
    print_colored("OK\n", VGA_COLOR_GREEN);
}

void init_interrupts(void) {
    print("Setting up interrupt handlers... ");
    
    // Initialize IDT (Interrupt Descriptor Table)
    init_idt();
    
    // Set up system call interface
    init_syscalls();
    
    // Enable interrupts
    asm volatile("sti");
    
    print_colored("OK\n", VGA_COLOR_GREEN);
}

void init_drivers(void) {
    print("Loading device drivers... ");
    
    // Initialize keyboard driver
    init_keyboard();
    
    // Initialize mouse driver
    init_mouse();
    
    // Initialize VGA driver
    init_vga();
    
    // Initialize disk driver
    init_disk();
    
    print_colored("OK\n", VGA_COLOR_GREEN);
}

void init_filesystem(void) {
    print("Mounting filesystems... ");
    
    // Initialize Virtual File System
    init_vfs();
    
    // Mount root filesystem
    mount_root_fs();
    
    // Create essential directories
    create_system_dirs();
    
    print_colored("OK\n", VGA_COLOR_GREEN);
}

void init_gui(void) {
    print("Initializing GUI subsystem... ");
    
    // Initialize graphics mode
    init_graphics_mode();
    
    // Initialize window manager
    init_window_manager();
    
    // Load desktop environment
    load_desktop();
    
    print_colored("OK\n", VGA_COLOR_GREEN);
}

void print(const char* str) {
    print_colored(str, VGA_COLOR_WHITE);
}

void print_colored(const char* str, int color) {
    while(*str) {
        if(*str == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            int index = (cursor_y * VGA_WIDTH + cursor_x) * 2;
            vga_buffer[index] = *str;
            vga_buffer[index + 1] = color;
            cursor_x++;
        }
        
        if(cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y++;
        }
        
        if(cursor_y >= VGA_HEIGHT) {
            // Scroll screen up
            for(int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i++) {
                vga_buffer[i] = vga_buffer[i + VGA_WIDTH * 2];
            }
            
            // Clear last line
            for(int i = (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i < VGA_HEIGHT * VGA_WIDTH * 2; i += 2) {
                vga_buffer[i] = ' ';
                vga_buffer[i + 1] = VGA_COLOR_WHITE;
            }
            
            cursor_y = VGA_HEIGHT - 1;
        }
        
        str++;
    }
    
    update_cursor();
}

void clear_screen(void) {
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        vga_buffer[i] = ' ';
        vga_buffer[i + 1] = VGA_COLOR_WHITE;
    }
    cursor_x = 0;
    cursor_y = 0;
    update_cursor();
}

void update_cursor(void) {
    int pos = cursor_y * VGA_WIDTH + cursor_x;
    
    // Send cursor position to VGA controller
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
}

// Port I/O functions
void outb(unsigned short port, unsigned char val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}