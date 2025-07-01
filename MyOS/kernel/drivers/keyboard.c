// kernel/drivers/keyboard.c
// Keyboard driver

#include "../include/kernel.h"

static int caps_lock = 0;
static int shift_pressed = 0;
static int ctrl_pressed = 0;
static int alt_pressed = 0;

void init_keyboard(void) {
    // Keyboard is initialized by setting up IRQ1 in interrupt.c
    print("Keyboard driver loaded\n");
}

// kernel/drivers/mouse.c
// Mouse driver

#include "../include/kernel.h"

typedef struct {
    int x, y;
    int buttons;
    int delta_x, delta_y;
} MouseState;

static MouseState mouse = {0, 0, 0, 0, 0};

void init_mouse(void) {
    // Enable auxiliary mouse device
    outb(0x64, 0xA8);
    
    // Enable mouse interrupts
    outb(0x64, 0x20);
    unsigned char status = inb(0x60) | 2;
    outb(0x64, 0x60);
    outb(0x60, status);
    
    // Set mouse to use default settings
    outb(0x64, 0xD4);
    outb(0x60, 0xF6);
    inb(0x60); // Acknowledge
    
    // Enable mouse
    outb(0x64, 0xD4);
    outb(0x60, 0xF4);
    inb(0x60); // Acknowledge
    
    print("Mouse driver loaded\n");
}

void handle_mouse_packet(unsigned char packet[3]) {
    mouse.buttons = packet[0] & 0x07;
    mouse.delta_x = packet[1];
    mouse.delta_y = packet[2];
    
    // Update position
    mouse.x += mouse.delta_x;
    mouse.y -= mouse.delta_y; // Invert Y axis
    
    // Clamp to screen bounds
    if(mouse.x < 0) mouse.x = 0;
    if(mouse.y < 0) mouse.y = 0;
    if(mouse.x >= SCREEN_WIDTH) mouse.x = SCREEN_WIDTH - 1;
    if(mouse.y >= SCREEN_HEIGHT) mouse.y = SCREEN_HEIGHT - 1;
    
    // Send to GUI system
    handle_mouse_input(mouse.x, mouse.y, mouse.buttons);
}

// kernel/drivers/vga.c
// VGA graphics driver

#include "../include/kernel.h"
#include "../include/graphics.h"

static unsigned int* vga_framebuffer = (unsigned int*)0xA0000000;
static int graphics_mode = 0;

void init_vga(void) {
    print("VGA driver loaded\n");
}

void set_graphics_mode(int mode) {
    if(mode == 1) {
        // Switch to VESA mode (simplified)
        graphics_mode = 1;
        vga_framebuffer = (unsigned int*)0xA0000000;
    } else {
        // Text mode
        graphics_mode = 0;
    }
}

void vga_put_pixel(int x, int y, unsigned int color) {
    if(graphics_mode && x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT) {
        vga_framebuffer[y * SCREEN_WIDTH + x] = color;
    }
}

// kernel/drivers/disk.c
// Disk driver (ATA/IDE)

#include "../include/kernel.h"

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

typedef struct {
    unsigned short io_base;
    unsigned short control_base;
    int drive_num;
} ATADrive;

static ATADrive primary_master = {ATA_PRIMARY_IO, 0x3F6, 0};
static ATADrive primary_slave = {ATA_PRIMARY_IO, 0x3F6, 1};

void init_disk(void) {
    // Reset ATA controller
    outb(primary_master.control_base, 0x04);
    for(volatile int i = 0; i < 1000; i++); // Delay
    outb(primary_master.control_base, 0x00);
    
    // Wait for drives to be ready
    while((inb(primary_master.io_base + 7) & 0x80) != 0);
    
    print("Disk driver loaded\n");
}

int ata_read_sectors(ATADrive* drive, unsigned int lba, unsigned char sectors, unsigned short* buffer) {
    // Select drive
    outb(drive->io_base + 6, 0xE0 | (drive->drive_num << 4) | ((lba >> 24) & 0x0F));
    
    // Send sector count
    outb(drive->io_base + 2, sectors);
    
    // Send LBA
    outb(drive->io_base + 3, lba & 0xFF);
    outb(drive->io_base + 4, (lba >> 8) & 0xFF);
    outb(drive->io_base + 5, (lba >> 16) & 0xFF);
    
    // Send read command
    outb(drive->io_base + 7, 0x20);
    
    // Wait for command to complete
    while((inb(drive->io_base + 7) & 0x80) != 0);
    
    // Check for errors
    if(inb(drive->io_base + 7) & 0x01) {
        return -1; // Error
    }
    
    // Read data
    for(int i = 0; i < sectors * 256; i++) {
        buffer[i] = inw(drive->io_base);
    }
    
    return 0;
}

int ata_write_sectors(ATADrive* drive, unsigned int lba, unsigned char sectors, unsigned short* buffer) {
    // Select drive
    outb(drive->io_base + 6, 0xE0 | (drive->drive_num << 4) | ((lba >> 24) & 0x0F));
    
    // Send sector count
    outb(drive->io_base + 2, sectors);
    
    // Send LBA
    outb(drive->io_base + 3, lba & 0xFF);
    outb(drive->io_base + 4, (lba >> 8) & 0xFF);
    outb(drive->io_base + 5, (lba >> 16) & 0xFF);
    
    // Send write command
    outb(drive->io_base + 7, 0x30);
    
    // Wait for drive to be ready
    while((inb(drive->io_base + 7) & 0x08) == 0);
    
    // Write data
    for(int i = 0; i < sectors * 256; i++) {
        outw(drive->io_base, buffer[i]);
    }
    
    // Wait for completion
    while((inb(drive->io_base + 7) & 0x80) != 0);
    
    return 0;
}

unsigned short inw(unsigned short port) {
    unsigned short result;
    asm volatile("inw %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

void outw(unsigned short port, unsigned short val) {
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

// kernel/drivers/network.c
// Network driver (basic implementation)

#include "../include/kernel.h"
#include "../include/network.h"

static NetworkInterface netif;

void init_network(void) {
    // Initialize network interface with default values
    netif.mac[0] = 0x52;
    netif.mac[1] = 0x54;
    netif.mac[2] = 0x00;
    netif.mac[3] = 0x12;
    netif.mac[4] = 0x34;
    netif.mac[5] = 0x56;
    
    netif.ip = 0xC0A80101; // 192.168.1.1
    netif.netmask = 0xFFFFFF00; // 255.255.255.0
    netif.gateway = 0xC0A80101; // 192.168.1.1
    
    print("Network driver loaded\n");
}

void send_ethernet_frame(unsigned char* dest_mac, unsigned short ethertype, unsigned char* data, unsigned short length) {
    // Ethernet frame structure:
    // [Destination MAC (6)] [Source MAC (6)] [EtherType (2)] [Data] [CRC (4)]
    
    unsigned char frame[1518]; // Maximum Ethernet frame size
    int pos = 0;
    
    // Destination MAC
    for(int i = 0; i < 6; i++) {
        frame[pos++] = dest_mac[i];
    }
    
    // Source MAC
    for(int i = 0; i < 6; i++) {
        frame[pos++] = netif.mac[i];
    }
    
    // EtherType
    frame[pos++] = (ethertype >> 8) & 0xFF;
    frame[pos++] = ethertype & 0xFF;
    
    // Data
    for(int i = 0; i < length; i++) {
        frame[pos++] = data[i];
    }
    
    // Send frame (implementation would depend on network hardware)
    // This is a placeholder
}

void process_received_frame(unsigned char* frame, unsigned short length) {
    if(length < 14) return; // Too short for Ethernet header
    
    // Extract header fields
    unsigned char dest_mac[6];
    unsigned char src_mac[6];
    unsigned short ethertype;
    
    for(int i = 0; i < 6; i++) {
        dest_mac[i] = frame[i];
        src_mac[i] = frame[i + 6];
    }
    
    ethertype = (frame[12] << 8) | frame[13];
    
    // Check if frame is for us
    int for_us = 1;
    for(int i = 0; i < 6; i++) {
        if(dest_mac[i] != netif.mac[i] && dest_mac[i] != 0xFF) {
            for_us = 0;
            break;
        }
    }
    
    if(!for_us) return;
    
    // Process based on EtherType
    switch(ethertype) {
        case 0x0800: // IPv4
            process_ip_packet(frame + 14, length - 14);
            break;
        case 0x0806: // ARP
            // Process ARP packet
            break;
        default:
            // Unknown protocol
            break;
    }
}

// kernel/core/timer.c
// Timer management

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