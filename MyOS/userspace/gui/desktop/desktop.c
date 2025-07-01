// userspace/gui/desktop/desktop.c
// Desktop Environment for MyOS

#include "../../lib/libgui/graphics.h"
#include "../../lib/libgui/window.h"
#include "../../../kernel/include/kernel.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define TASKBAR_HEIGHT 40
#define DESKTOP_COLOR 0x5588BB

typedef struct {
    int x, y;
    int width, height;
    char title[64];
    int active;
    int minimized;
} Window;

typedef struct {
    int x, y;
    int width, height;
    char text[32];
    void (*callback)(void);
} Button;

// Global variables
static unsigned int* framebuffer;
static Window windows[32];
static int window_count = 0;
static Button taskbar_buttons[16];
static int button_count = 0;
static int mouse_x = 512, mouse_y = 384;

// Function prototypes
void start_desktop_environment(void);
void init_graphics_mode(void);
void init_window_manager(void);
void load_desktop(void);
void draw_desktop(void);
void draw_taskbar(void);
void draw_window(Window* win);
void handle_mouse_input(int x, int y, int buttons);
void handle_keyboard_input(int scancode);
void create_window(const char* title, int x, int y, int width, int height);
void create_button(const char* text, int x, int y, int width, int height, void (*callback)(void));
void draw_pixel(int x, int y, unsigned int color);
void draw_rectangle(int x, int y, int width, int height, unsigned int color);
void draw_text(int x, int y, const char* text, unsigned int color);
void launch_file_manager(void);
void launch_text_editor(void);
void launch_calculator(void);
void launch_terminal(void);

void start_desktop_environment(void) {
    print("Starting desktop environment...\n");
    
    // Initialize graphics subsystem
    init_graphics_mode();
    init_window_manager();
    load_desktop();
    
    // Main GUI loop
    while(1) {
        draw_desktop();
        handle_input();
        update_windows();
        
        // Small delay
        for(volatile int i = 0; i < 100000; i++);
    }
}

void init_graphics_mode(void) {
    // Switch to VESA graphics mode (1024x768x32)
    // This is simplified - real implementation would use VESA BIOS
    framebuffer = (unsigned int*)0xA0000000; // Assume linear framebuffer
    
    // Clear screen
    for(int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        framebuffer[i] = DESKTOP_COLOR;
    }
}

void init_window_manager(void) {
    window_count = 0;
    button_count = 0;
    
    // Initialize window list
    for(int i = 0; i < 32; i++) {
        windows[i].active = 0;
    }
}

void load_desktop(void) {
    // Create taskbar buttons
    create_button("Files", 10, SCREEN_HEIGHT - TASKBAR_HEIGHT + 5, 60, 30, launch_file_manager);
    create_button("Editor", 80, SCREEN_HEIGHT - TASKBAR_HEIGHT + 5, 60, 30, launch_text_editor);
    create_button("Calc", 150, SCREEN_HEIGHT - TASKBAR_HEIGHT + 5, 60, 30, launch_calculator);
    create_button("Terminal", 220, SCREEN_HEIGHT - TASKBAR_HEIGHT + 5, 70, 30, launch_terminal);
    
    // Create welcome window
    create_window("Welcome to MyOS", 300, 200, 400, 300);
}

void draw_desktop(void) {
    // Draw desktop background
    draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT - TASKBAR_HEIGHT, DESKTOP_COLOR);
    
    // Draw taskbar
    draw_taskbar();
    
    // Draw all windows
    for(int i = 0; i < window_count; i++) {
        if(windows[i].active && !windows[i].minimized) {
            draw_window(&windows[i]);
        }
    }
    
    // Draw mouse cursor
    draw_rectangle(mouse_x, mouse_y, 10, 16, 0xFFFFFF);
    draw_rectangle(mouse_x + 1, mouse_y + 1, 8, 14, 0x000000);
}

void draw_taskbar(void) {
    // Draw taskbar background
    draw_rectangle(0, SCREEN_HEIGHT - TASKBAR_HEIGHT, SCREEN_WIDTH, TASKBAR_HEIGHT, 0x333333);
    
    // Draw taskbar buttons
    for(int i = 0; i < button_count; i++) {
        Button* btn = &taskbar_buttons[i];
        
        // Button background
        draw_rectangle(btn->x, btn->y, btn->width, btn->height, 0x666666);
        
        // Button border
        draw_rectangle(btn->x, btn->y, btn->width, 1, 0x999999);
        draw_rectangle(btn->x, btn->y, 1, btn->height, 0x999999);
        draw_rectangle(btn->x + btn->width - 1, btn->y, 1, btn->height, 0x333333);
        draw_rectangle(btn->x, btn->y + btn->height - 1, btn->width, 1, 0x333333);
        
        // Button text
        draw_text(btn->x + 5, btn->y + 8, btn->text, 0xFFFFFF);
    }
    
    // Draw system clock
    draw_text(SCREEN_WIDTH - 80, SCREEN_HEIGHT - 25, "12:34:56", 0xFFFFFF);
}

void draw_window(Window* win) {
    // Window background
    draw_rectangle(win->x, win->y, win->width, win->height, 0xE0E0E0);
    
    // Title bar
    draw_rectangle(win->x, win->y, win->width, 25, 0x0066CC);
    
    // Title text
    draw_text(win->x + 5, win->y + 5, win->title, 0xFFFFFF);
    
    // Close button
    draw_rectangle(win->x + win->width - 20, win->y + 3, 15, 15, 0xFF0000);
    draw_text(win->x + win->width - 17, win->y + 6, "X", 0xFFFFFF);
    
    // Window border
    draw_rectangle(win->x, win->y, win->width, 1, 0x000000);
    draw_rectangle(win->x, win->y, 1, win->height, 0x000000);
    draw_rectangle(win->x + win->width - 1, win->y, 1, win->height, 0x000000);
    draw_rectangle(win->x, win->y + win->height - 1, win->width, 1, 0x000000);
    
    // Window content area
    if(strcmp(win->title, "Welcome to MyOS") == 0) {
        draw_text(win->x + 20, win->y + 50, "Welcome to MyOS!", 0x000000);
        draw_text(win->x + 20, win->y + 80, "This is a simple operating system", 0x000000);
        draw_text(win->x + 20, win->y + 110, "built from scratch.", 0x000000);
        draw_text(win->x + 20, win->y + 150, "Features:", 0x000000);
        draw_text(win->x + 30, win->y + 180, "- Multi-tasking kernel", 0x000000);
        draw_text(win->x + 30, win->y + 200, "- GUI desktop environment", 0x000000);
        draw_text(win->x + 30, win->y + 220, "- File system support", 0x000000);
        draw_text(win->x + 30, win->y + 240, "- Network capabilities", 0x000000);
    }
}

void create_window(const char* title, int x, int y, int width, int height) {
    if(window_count < 32) {
        Window* win = &windows[window_count];
        strcpy(win->title, title);
        win->x = x;
        win->y = y;
        win->width = width;
        win->height = height;
        win->active = 1;
        win->minimized = 0;
        window_count++;
    }
}

void create_button(const char* text, int x, int y, int width, int height, void (*callback)(void)) {
    if(button_count < 16) {
        Button* btn = &taskbar_buttons[button_count];
        strcpy(btn->text, text);
        btn->x = x;
        btn->y = y;
        btn->width = width;
        btn->height = height;
        btn->callback = callback;
        button_count++;
    }
}

void draw_pixel(int x, int y, unsigned int color) {
    if(x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        framebuffer[y * SCREEN_WIDTH + x] = color;
    }
}

void draw_rectangle(int x, int y, int width, int height, unsigned int color) {
    for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
            draw_pixel(x + j, y + i, color);
        }
    }
}

void draw_text(int x, int y, const char* text, unsigned int color) {
    // Simple bitmap font rendering (8x8 pixels per character)
    // This is a simplified implementation
    static unsigned char font[256][8] = {
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
        // ... (font data would be here)
    };
    
    int char_x = x;
    while(*text) {
        unsigned char c = *text;
        for(int row = 0; row < 8; row++) {
            unsigned char pixel_row = font[c][row];
            for(int col = 0; col < 8; col++) {
                if(pixel_row & (1 << (7 - col))) {
                    draw_pixel(char_x + col, y + row, color);
                }
            }
        }
        char_x += 8;
        text++;
    }
}

// Application launchers
void launch_file_manager(void) {
    create_window("File Manager", 100, 100, 600, 400);
}

void launch_text_editor(void) {
    create_window("Text Editor", 150, 150, 500, 350);
}

void launch_calculator(void) {
    create_window("Calculator", 400, 300, 200, 250);
}

void launch_terminal(void) {
    create_window("Terminal", 200, 200, 500, 300);
}