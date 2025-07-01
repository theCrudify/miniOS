// kernel/core/syscall.c
// System call implementation

#include "../include/kernel.h"
#include "../include/filesystem.h"
#include "../include/process.h"

// System call numbers
#define SYS_EXIT 0
#define SYS_WRITE 1
#define SYS_READ 2
#define SYS_OPEN 3
#define SYS_CLOSE 4
#define SYS_FORK 5
#define SYS_EXEC 6
#define SYS_GETPID 7
#define SYS_SLEEP 8
#define SYS_MALLOC 9
#define SYS_FREE 10

// System call table
typedef int (*syscall_func_t)(int, int, int);

static syscall_func_t syscall_table[] = {
    sys_exit,     // 0
    sys_write,    // 1
    sys_read,     // 2
    sys_open,     // 3
    sys_close,    // 4
    sys_fork,     // 5
    sys_exec,     // 6
    sys_getpid,   // 7
    sys_sleep,    // 8
    sys_malloc,   // 9
    sys_free      // 10
};

#define NUM_SYSCALLS (sizeof(syscall_table) / sizeof(syscall_func_t))

// System call handler
int syscall_handler(int num, int arg1, int arg2, int arg3) {
    if(num >= 0 && num < NUM_SYSCALLS) {
        return syscall_table[num](arg1, arg2, arg3);
    }
    return -1; // Invalid system call
}

void init_syscalls(void) {
    // Set up system call interrupt (interrupt 0x80)
    set_idt_gate(0x80, (unsigned int)syscall_interrupt);
    print("System calls initialized\n");
}

// System call implementations

int sys_exit(int status, int unused1, int unused2) {
    // Terminate current process
    Process* current = get_current_process();
    if(current) {
        destroy_process(current->pid);
    }
    return 0;
}

int sys_write(int fd, int buffer, int size) {
    char* buf = (char*)buffer;
    
    if(fd == 1 || fd == 2) { // stdout or stderr
        for(int i = 0; i < size; i++) {
            char str[2] = {buf[i], '\0'};
            print(str);
        }
        return size;
    }
    
    // File writing would go here
    return -1;
}

int sys_read(int fd, int buffer, int size) {
    char* buf = (char*)buffer;
    
    if(fd == 0) { // stdin
        for(int i = 0; i < size; i++) {
            buf[i] = getchar();
            if(buf[i] == '\n') {
                return i + 1;
            }
        }
        return size;
    }
    
    // File reading would go here
    return -1;
}

int sys_open(int filename, int flags, int mode) {
    // File opening implementation
    return -1; // Not implemented yet
}

int sys_close(int fd, int unused1, int unused2) {
    // File closing implementation
    return -1; // Not implemented yet
}

int sys_fork(int unused1, int unused2, int unused3) {
    // Process forking implementation
    return -1; // Not implemented yet
}

int sys_exec(int filename, int argv, int envp) {
    // Process execution implementation
    return -1; // Not implemented yet
}

int sys_getpid(int unused1, int unused2, int unused3) {
    Process* current = get_current_process();
    return current ? current->pid : -1;
}

int sys_sleep(int milliseconds, int unused1, int unused2) {
    sleep(milliseconds);
    return 0;
}

int sys_malloc(int size, int unused1, int unused2) {
    return (int)kmalloc(size);
}

int sys_free(int ptr, int unused1, int unused2) {
    kfree((void*)ptr);
    return 0;
}

// User space system call interface
int syscall(int num, int arg1, int arg2, int arg3) {
    int result;
    asm volatile("int $0x80" 
                 : "=a"(result) 
                 : "a"(num), "b"(arg1), "c"(arg2), "d"(arg3)
                 : "memory");
    return result;
}

// kernel/process/scheduler.c
// Process scheduler implementation

#include "../include/kernel.h"
#include "../include/process.h"
#include "../include/memory.h"

static Process* process_list = NULL;
static Process* current_process = NULL;
static int next_pid = 1;

void init_scheduler(void) {
    process_list = NULL;
    current_process = NULL;
    
    // Create kernel process
    Process* kernel_proc = create_process("kernel", NULL);
    kernel_proc->state = PROC_RUNNING;
    current_process = kernel_proc;
    
    print("Process scheduler initialized\n");
}

Process* create_process(const char* name, void* entry_point) {
    Process* proc = (Process*)kmalloc(sizeof(Process));
    if(!proc) return NULL;
    
    proc->pid = next_pid++;
    proc->state = PROC_READY;
    proc->esp = 0;
    proc->ebp = 0;
    proc->page_directory = 0; // Would set up page directory
    proc->next = process_list;
    strcpy(proc->name, name);
    
    process_list = proc;
    
    return proc;
}

void destroy_process(uint32_t pid) {
    Process* current = process_list;
    Process* prev = NULL;
    
    while(current) {
        if(current->pid == pid) {
            if(prev) {
                prev->next = current->next;
            } else {
                process_list = current->next;
            }
            
            // Free process memory
            kfree(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void schedule(void) {
    if(!current_process || !process_list) return;
    
    // Simple round-robin scheduler
    Process* next = current_process->next;
    if(!next) next = process_list;
    
    // Find next ready process
    while(next && next->state != PROC_READY && next != current_process) {
        next = next->next;
        if(!next) next = process_list;
    }
    
    if(next && next != current_process) {
        current_process->state = PROC_READY;
        current_process = next;
        current_process->state = PROC_RUNNING;
        
        // Context switch would happen here
    }
}

Process* get_current_process(void) {
    return current_process;
}

void yield(void) {
    schedule();
}

void schedule_processes(void) {
    // Called from timer interrupt
    schedule();
}

// kernel/process/thread.c
// Thread management

#include "../include/kernel.h"
#include "../include/process.h"

static Thread* thread_list = NULL;
static int next_tid = 1;

Thread* create_thread(Process* proc, void* entry_point) {
    Thread* thread = (Thread*)kmalloc(sizeof(Thread));
    if(!thread) return NULL;
    
    thread->tid = next_tid++;
    thread->state = PROC_READY;
    thread->esp = 0;
    thread->ebp = 0;
    thread->process = proc;
    thread->next = thread_list;
    
    thread_list = thread;
    
    return thread;
}

void destroy_thread(uint32_t tid) {
    Thread* current = thread_list;
    Thread* prev = NULL;
    
    while(current) {
        if(current->tid == tid) {
            if(prev) {
                prev->next = current->next;
            } else {
                thread_list = current->next;
            }
            
            kfree(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// kernel/process/ipc.c
// Inter-process communication

#include "../include/kernel.h"
#include "../include/process.h"

typedef struct message {
    uint32_t sender_pid;
    uint32_t receiver_pid;
    uint32_t type;
    uint32_t length;
    void* data;
    struct message* next;
} Message;

static Message* message_queue = NULL;

int send_message(uint32_t dest_pid, uint32_t type, void* data, uint32_t length) {
    Message* msg = (Message*)kmalloc(sizeof(Message));
    if(!msg) return -1;
    
    msg->sender_pid = get_current_process()->pid;
    msg->receiver_pid = dest_pid;
    msg->type = type;
    msg->length = length;
    msg->data = kmalloc(length);
    
    if(!msg->data) {
        kfree(msg);
        return -1;
    }
    
    memcpy(msg->data, data, length);
    
    // Add to message queue
    msg->next = message_queue;
    message_queue = msg;
    
    return 0;
}

int receive_message(uint32_t* sender_pid, uint32_t* type, void* buffer, uint32_t buffer_size) {
    uint32_t current_pid = get_current_process()->pid;
    Message* msg = message_queue;
    Message* prev = NULL;
    
    // Find message for current process
    while(msg) {
        if(msg->receiver_pid == current_pid) {
            // Found message
            *sender_pid = msg->sender_pid;
            *type = msg->type;
            
            uint32_t copy_size = (msg->length < buffer_size) ? msg->length : buffer_size;
            memcpy(buffer, msg->data, copy_size);
            
            // Remove from queue
            if(prev) {
                prev->next = msg->next;
            } else {
                message_queue = msg->next;
            }
            
            kfree(msg->data);
            kfree(msg);
            
            return copy_size;
        }
        prev = msg;
        msg = msg->next;
    }
    
    return -1; // No message found
}