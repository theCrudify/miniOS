// kernel/include/kernel.h
// Main kernel header file

#ifndef KERNEL_H
#define KERNEL_H

// Basic types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;

typedef uint32_t size_t;

// Boolean type
typedef enum { false = 0, true = 1 } bool;

// NULL definition
#define NULL ((void*)0)

// Function prototypes
void kernel_main(void);
void print(const char* str);
void print_colored(const char* str, int color);
void clear_screen(void);
void outb(unsigned short port, unsigned char val);
unsigned char inb(unsigned short port);

// System call interface
int syscall(int num, int arg1, int arg2, int arg3);

// Process management
void schedule_processes(void);
void handle_interrupts(void);
void update_gui(void);

// Memory functions
void* kmalloc_early(unsigned int size);

// String functions
int strlen(const char* str);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
char* strtok(char* str, const char* delim);
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
void* realloc(void* ptr, size_t size);

#endif // KERNEL_H

// kernel/include/memory.h
// Memory management header

#ifndef MEMORY_H
#define MEMORY_H

#include "kernel.h"

// Memory management functions
void init_physical_memory(void);
void init_paging(void);
void init_heap(void);
void* malloc(unsigned int size);
void free(void* ptr);
void* kmalloc(unsigned int size);
void kfree(void* ptr);

// Physical memory
unsigned int allocate_physical_page(void);
void free_physical_page(unsigned int page);

// Virtual memory
void map_page(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags);
void unmap_page(unsigned int virtual_addr);
void enable_paging(void);

// Memory information
unsigned int get_total_memory(void);
unsigned int get_free_memory(void);
unsigned int get_used_memory(void);
unsigned int get_heap_usage(void);

#endif // MEMORY_H

// kernel/include/process.h
// Process management header

#ifndef PROCESS_H
#define PROCESS_H

#include "kernel.h"

// Process states
#define PROC_RUNNING 1
#define PROC_READY 2
#define PROC_BLOCKED 3
#define PROC_TERMINATED 4

// Process structure
typedef struct process {
    uint32_t pid;
    uint32_t state;
    uint32_t esp;
    uint32_t ebp;
    uint32_t page_directory;
    struct process* next;
    char name[64];
} Process;

// Process management functions
void init_scheduler(void);
Process* create_process(const char* name, void* entry_point);
void destroy_process(uint32_t pid);
void schedule(void);
Process* get_current_process(void);
void yield(void);

// Thread management
typedef struct thread {
    uint32_t tid;
    uint32_t state;
    uint32_t esp;
    uint32_t ebp;
    Process* process;
    struct thread* next;
} Thread;

Thread* create_thread(Process* proc, void* entry_point);
void destroy_thread(uint32_t tid);

#endif // PROCESS_H

// kernel/include/filesystem.h
// File system header

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "kernel.h"

// Forward declaration
struct vfs_node;

// File system functions
void init_vfs(void);
void mount_root_fs(void);
void create_system_dirs(void);

// File operations
struct vfs_node* create_file(const char* name, unsigned int type);
struct vfs_node* create_directory(const char* name);
struct vfs_node* find_file(const char* path);
int read_file(struct vfs_node* node, unsigned int offset, unsigned int size, char* buffer);
int write_file(struct vfs_node* node, unsigned int offset, unsigned int size, char* buffer);
int delete_file(const char* path);
int copy_file(const char* src, const char* dest);
int move_file(const char* src, const char* dest);

// Directory operations
char** list_directory(const char* path);
int change_directory(const char* path);
char* get_current_directory(void);

#endif // FILESYSTEM_H

// kernel/include/graphics.h
// Graphics and GUI header

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "kernel.h"

// Screen dimensions
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

// Color definitions
#define COLOR_BLACK 0x000000
#define COLOR_WHITE 0xFFFFFF
#define COLOR_RED 0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE 0x0000FF
#define COLOR_YELLOW 0xFFFF00
#define COLOR_CYAN 0x00FFFF
#define COLOR_MAGENTA 0xFF00FF
#define COLOR_GRAY 0x808080
#define COLOR_LIGHT_GRAY 0xC0C0C0
#define COLOR_DARK_GRAY 0x404040

// Graphics functions
void init_graphics_mode(void);
void draw_pixel(int x, int y, unsigned int color);
void draw_rectangle(int x, int y, int width, int height, unsigned int color);
void draw_line(int x1, int y1, int x2, int y2, unsigned int color);
void draw_circle(int x, int y, int radius, unsigned int color);
void draw_text(int x, int y, const char* text, unsigned int color);

// GUI functions
void start_desktop_environment(void);
void init_window_manager(void);
void load_desktop(void);
void draw_desktop(void);
void handle_input(void);
void update_windows(void);

// Input handling
void handle_mouse_input(int x, int y, int buttons);
void handle_keyboard_input(int scancode);

#endif // GRAPHICS_H

// kernel/include/network.h
// Network subsystem header

#ifndef NETWORK_H
#define NETWORK_H

#include "kernel.h"

// Network protocols
#define PROTO_ETHERNET 1
#define PROTO_IP 2
#define PROTO_TCP 3
#define PROTO_UDP 4

// Network structures
typedef struct {
    uint8_t mac[6];
    uint32_t ip;
    uint32_t netmask;
    uint32_t gateway;
} NetworkInterface;

typedef struct {
    uint32_t src_ip;
    uint32_t dest_ip;
    uint16_t src_port;
    uint16_t dest_port;
    uint8_t protocol;
    uint16_t length;
    uint8_t* data;
} NetworkPacket;

// Network functions
void init_network(void);
void init_ethernet(void);
void init_ip_stack(void);
void init_tcp_stack(void);
void init_udp_stack(void);

// Packet handling
void send_packet(NetworkPacket* packet);
void receive_packet(NetworkPacket* packet);
void process_ethernet_frame(uint8_t* frame, uint16_t length);
void process_ip_packet(uint8_t* packet, uint16_t length);

// Socket interface
int socket(int domain, int type, int protocol);
int bind(int sockfd, uint32_t addr, uint16_t port);
int listen(int sockfd, int backlog);
int accept(int sockfd);
int connect(int sockfd, uint32_t addr, uint16_t port);
int send(int sockfd, const void* buf, size_t len, int flags);
int recv(int sockfd, void* buf, size_t len, int flags);
int close_socket(int sockfd);

#endif // NETWORK_H

// userspace/lib/libgui/graphics.h
// User space graphics library

#ifndef LIBGUI_GRAPHICS_H
#define LIBGUI_GRAPHICS_H

// Graphics primitives for user applications
void gui_draw_pixel(int x, int y, unsigned int color);
void gui_draw_rectangle(int x, int y, int width, int height, unsigned int color);
void gui_draw_text(int x, int y, const char* text, unsigned int color);
void gui_draw_image(int x, int y, int width, int height, unsigned int* image_data);

// Event handling
typedef struct {
    int type;
    int x, y;
    int button;
    int key;
} GUIEvent;

#define EVENT_MOUSE_CLICK 1
#define EVENT_MOUSE_MOVE 2
#define EVENT_KEY_PRESS 3
#define EVENT_KEY_RELEASE 4

int gui_get_event(GUIEvent* event);
void gui_flush(void);

#endif // LIBGUI_GRAPHICS_H

// userspace/lib/libgui/window.h
// Window management library

#ifndef LIBGUI_WINDOW_H
#define LIBGUI_WINDOW_H

#include "graphics.h"

typedef struct window {
    int id;
    int x, y;
    int width, height;
    char title[64];
    unsigned int* buffer;
    int visible;
    int focused;
    struct window* next;
} Window;

// Window functions
Window* create_window(const char* title, int x, int y, int width, int height);
void destroy_window(Window* window);
void show_window(Window* window);
void hide_window(Window* window);
void move_window(Window* window, int x, int y);
void resize_window(Window* window, int width, int height);
void set_window_title(Window* window, const char* title);
void draw_window_content(Window* window);
void refresh_window(Window* window);

// Window events
void handle_window_event(Window* window, GUIEvent* event);

#endif // LIBGUI_WINDOW_H