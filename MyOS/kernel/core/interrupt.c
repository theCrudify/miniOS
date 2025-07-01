// kernel/core/interrupt.c
// Interrupt handling system

#include "../include/kernel.h"

// IDT structure
typedef struct {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short offset_high;
} __attribute__((packed)) IDTEntry;

typedef struct {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) IDTDescriptor;

// IDT with 256 entries
static IDTEntry idt[256];
static IDTDescriptor idt_desc;

// Exception messages
static const char* exception_messages[] = {
    "Divide by Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check"
};

// Function prototypes
void init_idt(void);
void set_idt_gate(int n, unsigned int handler);
void exception_handler(int exception_num);
void irq_handler(int irq_num);
void timer_handler(void);
void keyboard_handler(void);

// External assembly interrupt handlers
extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

void init_idt(void) {
    idt_desc.limit = sizeof(idt) - 1;
    idt_desc.base = (unsigned int)&idt;
    
    // Clear IDT
    for(int i = 0; i < 256; i++) {
        idt[i].offset_low = 0;
        idt[i].selector = 0;
        idt[i].zero = 0;
        idt[i].type_attr = 0;
        idt[i].offset_high = 0;
    }
    
    // Install exception handlers (ISR 0-31)
    set_idt_gate(0, (unsigned int)isr0);
    set_idt_gate(1, (unsigned int)isr1);
    set_idt_gate(2, (unsigned int)isr2);
    set_idt_gate(3, (unsigned int)isr3);
    set_idt_gate(4, (unsigned int)isr4);
    set_idt_gate(5, (unsigned int)isr5);
    set_idt_gate(6, (unsigned int)isr6);
    set_idt_gate(7, (unsigned int)isr7);
    set_idt_gate(8, (unsigned int)isr8);
    set_idt_gate(9, (unsigned int)isr9);
    set_idt_gate(10, (unsigned int)isr10);
    set_idt_gate(11, (unsigned int)isr11);
    set_idt_gate(12, (unsigned int)isr12);
    set_idt_gate(13, (unsigned int)isr13);
    set_idt_gate(14, (unsigned int)isr14);
    set_idt_gate(15, (unsigned int)isr15);
    set_idt_gate(16, (unsigned int)isr16);
    set_idt_gate(17, (unsigned int)isr17);
    set_idt_gate(18, (unsigned int)isr18);
    set_idt_gate(19, (unsigned int)isr19);
    set_idt_gate(20, (unsigned int)isr20);
    set_idt_gate(21, (unsigned int)isr21);
    set_idt_gate(22, (unsigned int)isr22);
    set_idt_gate(23, (unsigned int)isr23);
    set_idt_gate(24, (unsigned int)isr24);
    set_idt_gate(25, (unsigned int)isr25);
    set_idt_gate(26, (unsigned int)isr26);
    set_idt_gate(27, (unsigned int)isr27);
    set_idt_gate(28, (unsigned int)isr28);
    set_idt_gate(29, (unsigned int)isr29);
    set_idt_gate(30, (unsigned int)isr30);
    set_idt_gate(31, (unsigned int)isr31);
    
    // Remap PIC
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
    
    // Install IRQ handlers (IRQ 0-15)
    set_idt_gate(32, (unsigned int)irq0);   // Timer
    set_idt_gate(33, (unsigned int)irq1);   // Keyboard
    set_idt_gate(34, (unsigned int)irq2);
    set_idt_gate(35, (unsigned int)irq3);
    set_idt_gate(36, (unsigned int)irq4);
    set_idt_gate(37, (unsigned int)irq5);
    set_idt_gate(38, (unsigned int)irq6);
    set_idt_gate(39, (unsigned int)irq7);
    set_idt_gate(40, (unsigned int)irq8);
    set_idt_gate(41, (unsigned int)irq9);
    set_idt_gate(42, (unsigned int)irq10);
    set_idt_gate(43, (unsigned int)irq11);
    set_idt_gate(44, (unsigned int)irq12);  // Mouse
    set_idt_gate(45, (unsigned int)irq13);
    set_idt_gate(46, (unsigned int)irq14);  // Primary ATA
    set_idt_gate(47, (unsigned int)irq15);  // Secondary ATA
    
    // Load IDT
    asm volatile("lidt %0" : : "m"(idt_desc));
}

void set_idt_gate(int n, unsigned int handler) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = 0x08; // Code segment
    idt[n].zero = 0;
    idt[n].type_attr = 0x8E; // Present, Ring 0, 32-bit interrupt gate
    idt[n].offset_high = (handler >> 16) & 0xFFFF;
}

void exception_handler(int exception_num) {
    print_colored("EXCEPTION: ", VGA_COLOR_RED);
    if(exception_num < 19) {
        print_colored(exception_messages[exception_num], VGA_COLOR_RED);
    } else {
        print_colored("Reserved Exception", VGA_COLOR_RED);
    }
    print_colored("\nSystem Halted!", VGA_COLOR_RED);
    
    // Halt the system
    while(1) {
        asm volatile("hlt");
    }
}

void irq_handler(int irq_num) {
    switch(irq_num) {
        case 0:
            timer_handler();
            break;
        case 1:
            keyboard_handler();
            break;
        case 12:
            // Mouse handler would go here
            break;
        default:
            // Unhandled IRQ
            break;
    }
    
    // Send EOI to PIC
    if(irq_num >= 8) {
        outb(0xA0, 0x20); // Send EOI to slave PIC
    }
    outb(0x20, 0x20); // Send EOI to master PIC
}

static unsigned int timer_ticks = 0;

void timer_handler(void) {
    timer_ticks++;
    
    // Call scheduler every 10ms (100 Hz)
    if(timer_ticks % 10 == 0) {
        schedule_processes();
    }
}

static char key_buffer[256];
static int key_buffer_pos = 0;

void keyboard_handler(void) {
    unsigned char scancode = inb(0x60);
    
    // Simple scancode to ASCII conversion
    static char keymap[128] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    
    if(scancode < 128 && keymap[scancode]) {
        char key = keymap[scancode];
        
        // Add to key buffer
        if(key_buffer_pos < 255) {
            key_buffer[key_buffer_pos++] = key;
            key_buffer[key_buffer_pos] = '\0';
        }
        
        // Echo character (simple implementation)
        if(key >= 32 && key <= 126) {
            char str[2] = {key, '\0'};
            print(str);
        } else if(key == '\n') {
            print("\n");
        } else if(key == '\b') {
            // Handle backspace
            if(key_buffer_pos > 0) {
                key_buffer_pos--;
                key_buffer[key_buffer_pos] = '\0';
            }
        }
    }
}

char getchar(void) {
    while(key_buffer_pos == 0) {
        asm volatile("hlt"); // Wait for interrupt
    }
    
    char c = key_buffer[0];
    
    // Shift buffer
    for(int i = 0; i < key_buffer_pos - 1; i++) {
        key_buffer[i] = key_buffer[i + 1];
    }
    key_buffer_pos--;
    key_buffer[key_buffer_pos] = '\0';
    
    return c;
}

void handle_interrupts(void) {
    // This function is called from the main kernel loop
    // It can handle any deferred interrupt processing
}

// kernel/core/interrupt_asm.asm
// Assembly interrupt handlers (this would be in a separate .asm file)