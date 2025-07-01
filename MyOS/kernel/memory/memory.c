// kernel/memory/memory.c
// Memory management system for MyOS

#include "../include/memory.h"
#include "../include/kernel.h"

#define PAGE_SIZE 4096
#define HEAP_START 0x100000
#define HEAP_SIZE 0x1000000  // 16MB heap
#define MAX_PAGES 1024

// Physical memory management
typedef struct {
    unsigned int* bitmap;
    unsigned int total_pages;
    unsigned int free_pages;
    unsigned int used_pages;
} PhysicalMemoryManager;

// Virtual memory (paging)
typedef struct {
    unsigned int* page_directory;
    unsigned int* page_tables[1024];
} VirtualMemoryManager;

// Heap management
typedef struct heap_block {
    unsigned int size;
    int free;
    struct heap_block* next;
    struct heap_block* prev;
} HeapBlock;

typedef struct {
    HeapBlock* first_block;
    unsigned int total_size;
    unsigned int used_size;
} HeapManager;

// Global memory managers
static PhysicalMemoryManager pmm;
static VirtualMemoryManager vmm;
static HeapManager heap;

// Function prototypes
void init_physical_memory(void);
void init_paging(void);
void init_heap(void);
void* kmalloc(unsigned int size);
void kfree(void* ptr);
void* malloc(unsigned int size);
void free(void* ptr);
unsigned int allocate_physical_page(void);
void free_physical_page(unsigned int page);
void map_page(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags);
void unmap_page(unsigned int virtual_addr);
void enable_paging(void);

void init_physical_memory(void) {
    // Initialize physical memory bitmap
    // Assume 128MB of RAM for this example
    pmm.total_pages = (128 * 1024 * 1024) / PAGE_SIZE;
    pmm.free_pages = pmm.total_pages;
    pmm.used_pages = 0;
    
    // Allocate bitmap (1 bit per page)
    pmm.bitmap = (unsigned int*)0x200000; // Fixed location for bitmap
    
    // Clear bitmap (all pages free initially)
    for(unsigned int i = 0; i < (pmm.total_pages / 32) + 1; i++) {
        pmm.bitmap[i] = 0;
    }
    
    // Mark first 1MB as used (kernel space)
    for(unsigned int i = 0; i < 256; i++) {
        allocate_physical_page();
    }
}

void init_paging(void) {
    // Allocate page directory
    vmm.page_directory = (unsigned int*)kmalloc_early(PAGE_SIZE);
    
    // Clear page directory
    for(int i = 0; i < 1024; i++) {
        vmm.page_directory[i] = 0;
        vmm.page_tables[i] = 0;
    }
    
    // Identity map first 4MB (kernel space)
    for(unsigned int addr = 0; addr < 0x400000; addr += PAGE_SIZE) {
        map_page(addr, addr, 0x03); // Present + Read/Write
    }
    
    enable_paging();
}

void init_heap(void) {
    heap.first_block = (HeapBlock*)HEAP_START;
    heap.total_size = HEAP_SIZE;
    heap.used_size = sizeof(HeapBlock);
    
    // Initialize first block
    heap.first_block->size = HEAP_SIZE - sizeof(HeapBlock);
    heap.first_block->free = 1;
    heap.first_block->next = 0;
    heap.first_block->prev = 0;
}

void* kmalloc(unsigned int size) {
    return malloc(size);
}

void kfree(void* ptr) {
    free(ptr);
}

void* malloc(unsigned int size) {
    if(size == 0) return 0;
    
    // Align size to 4 bytes
    size = (size + 3) & ~3;
    
    HeapBlock* current = heap.first_block;
    
    // Find suitable free block
    while(current) {
        if(current->free && current->size >= size) {
            // Split block if it's much larger
            if(current->size > size + sizeof(HeapBlock) + 16) {
                HeapBlock* new_block = (HeapBlock*)((char*)current + sizeof(HeapBlock) + size);
                new_block->size = current->size - size - sizeof(HeapBlock);
                new_block->free = 1;
                new_block->next = current->next;
                new_block->prev = current;
                
                if(current->next) {
                    current->next->prev = new_block;
                }
                current->next = new_block;
                current->size = size;
            }
            
            current->free = 0;
            heap.used_size += current->size;
            return (void*)((char*)current + sizeof(HeapBlock));
        }
        current = current->next;
    }
    
    return 0; // Out of memory
}

void free(void* ptr) {
    if(!ptr) return;
    
    HeapBlock* block = (HeapBlock*)((char*)ptr - sizeof(HeapBlock));
    block->free = 1;
    heap.used_size -= block->size;
    
    // Merge with next block if it's free
    if(block->next && block->next->free) {
        block->size += block->next->size + sizeof(HeapBlock);
        if(block->next->next) {
            block->next->next->prev = block;
        }
        block->next = block->next->next;
    }
    
    // Merge with previous block if it's free
    if(block->prev && block->prev->free) {
        block->prev->size += block->size + sizeof(HeapBlock);
        if(block->next) {
            block->next->prev = block->prev;
        }
        block->prev->next = block->next;
    }
}

unsigned int allocate_physical_page(void) {
    for(unsigned int i = 0; i < pmm.total_pages; i++) {
        unsigned int byte_index = i / 32;
        unsigned int bit_index = i % 32;
        
        if(!(pmm.bitmap[byte_index] & (1 << bit_index))) {
            // Mark page as used
            pmm.bitmap[byte_index] |= (1 << bit_index);
            pmm.free_pages--;
            pmm.used_pages++;
            return i * PAGE_SIZE;
        }
    }
    return 0; // Out of physical memory
}

void free_physical_page(unsigned int page) {
    unsigned int page_index = page / PAGE_SIZE;
    unsigned int byte_index = page_index / 32;
    unsigned int bit_index = page_index % 32;
    
    // Mark page as free
    pmm.bitmap[byte_index] &= ~(1 << bit_index);
    pmm.free_pages++;
    pmm.used_pages--;
}

void map_page(unsigned int virtual_addr, unsigned int physical_addr, unsigned int flags) {
    unsigned int page_dir_index = virtual_addr >> 22;
    unsigned int page_table_index = (virtual_addr >> 12) & 0x3FF;
    
    // Check if page table exists
    if(!(vmm.page_directory[page_dir_index] & 0x01)) {
        // Create new page table
        unsigned int page_table_phys = allocate_physical_page();
        vmm.page_tables[page_dir_index] = (unsigned int*)page_table_phys;
        vmm.page_directory[page_dir_index] = page_table_phys | 0x03;
        
        // Clear page table
        for(int i = 0; i < 1024; i++) {
            vmm.page_tables[page_dir_index][i] = 0;
        }
    }
    
    // Set page table entry
    vmm.page_tables[page_dir_index][page_table_index] = physical_addr | flags;
}

void unmap_page(unsigned int virtual_addr) {
    unsigned int page_dir_index = virtual_addr >> 22;
    unsigned int page_table_index = (virtual_addr >> 12) & 0x3FF;
    
    if(vmm.page_directory[page_dir_index] & 0x01) {
        vmm.page_tables[page_dir_index][page_table_index] = 0;
        
        // Invalidate TLB entry
        asm volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
    }
}

void enable_paging(void) {
    // Load page directory
    asm volatile("mov %0, %%cr3" :: "r"(vmm.page_directory));
    
    // Enable paging
    unsigned int cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

// Early malloc for kernel initialization (before heap is set up)
static unsigned int early_malloc_ptr = 0x300000;

void* kmalloc_early(unsigned int size) {
    void* ptr = (void*)early_malloc_ptr;
    early_malloc_ptr += (size + 3) & ~3; // Align to 4 bytes
    return ptr;
}

// Memory information functions
unsigned int get_total_memory(void) {
    return pmm.total_pages * PAGE_SIZE;
}

unsigned int get_free_memory(void) {
    return pmm.free_pages * PAGE_SIZE;
}

unsigned int get_used_memory(void) {
    return pmm.used_pages * PAGE_SIZE;
}

unsigned int get_heap_usage(void) {
    return heap.used_size;
}