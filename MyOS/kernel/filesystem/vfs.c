// kernel/filesystem/vfs.c
// Virtual File System for MyOS

#include "../include/filesystem.h"
#include "../include/memory.h"
#include "../include/kernel.h"

#define MAX_FILES 1024
#define MAX_PATH_LENGTH 256
#define MAX_FILENAME_LENGTH 64

// File types
#define FILE_TYPE_REGULAR 1
#define FILE_TYPE_DIRECTORY 2
#define FILE_TYPE_DEVICE 3

// File permissions
#define PERM_READ 0x01
#define PERM_WRITE 0x02
#define PERM_EXECUTE 0x04

typedef struct vfs_node {
    char name[MAX_FILENAME_LENGTH];
    unsigned int type;
    unsigned int permissions;
    unsigned int size;
    unsigned int inode;
    struct vfs_node* parent;
    struct vfs_node* children;
    struct vfs_node* next;
    void* data;
    
    // Function pointers for operations
    int (*read)(struct vfs_node* node, unsigned int offset, unsigned int size, char* buffer);
    int (*write)(struct vfs_node* node, unsigned int offset, unsigned int size, char* buffer);
    int (*open)(struct vfs_node* node);
    int (*close)(struct vfs_node* node);
    struct vfs_node* (*readdir)(struct vfs_node* node, unsigned int index);
    struct vfs_node* (*finddir)(struct vfs_node* node, char* name);
} VFSNode;

typedef struct {
    VFSNode* root;
    VFSNode* current_dir;
    VFSNode nodes[MAX_FILES];
    int node_count;
} VFSManager;

// Global VFS manager
static VFSManager vfs;

// Function prototypes
void init_vfs(void);
void mount_root_fs(void);
void create_system_dirs(void);
VFSNode* create_file(const char* name, unsigned int type);
VFSNode* create_directory(const char* name);
int add_child(VFSNode* parent, VFSNode* child);
VFSNode* find_file(const char* path);
int read_file(VFSNode* node, unsigned int offset, unsigned int size, char* buffer);
int write_file(VFSNode* node, unsigned int offset, unsigned int size, char* buffer);
int delete_file(const char* path);
int copy_file(const char* src, const char* dest);
int move_file(const char* src, const char* dest);
char** list_directory(const char* path);
int change_directory(const char* path);
char* get_current_directory(void);

// Default file operations
int default_read(VFSNode* node, unsigned int offset, unsigned int size, char* buffer);
int default_write(VFSNode* node, unsigned int offset, unsigned int size, char* buffer);
int default_open(VFSNode* node);
int default_close(VFSNode* node);
VFSNode* default_readdir(VFSNode* node, unsigned int index);
VFSNode* default_finddir(VFSNode* node, char* name);

void init_vfs(void) {
    vfs.node_count = 0;
    vfs.root = NULL;
    vfs.current_dir = NULL;
    
    // Clear node array
    for(int i = 0; i < MAX_FILES; i++) {
        vfs.nodes[i].name[0] = '\0';
        vfs.nodes[i].type = 0;
        vfs.nodes[i].size = 0;
        vfs.nodes[i].inode = 0;
        vfs.nodes[i].parent = NULL;
        vfs.nodes[i].children = NULL;
        vfs.nodes[i].next = NULL;
        vfs.nodes[i].data = NULL;
    }
}

void mount_root_fs(void) {
    // Create root directory
    vfs.root = create_directory("/");
    vfs.current_dir = vfs.root;
    
    if(!vfs.root) {
        print("Failed to create root directory!\n");
        return;
    }
}

void create_system_dirs(void) {
    // Create essential system directories
    VFSNode* bin = create_directory("bin");
    VFSNode* usr = create_directory("usr");
    VFSNode* etc = create_directory("etc");
    VFSNode* tmp = create_directory("tmp");
    VFSNode* home = create_directory("home");
    VFSNode* dev = create_directory("dev");
    
    add_child(vfs.root, bin);
    add_child(vfs.root, usr);
    add_child(vfs.root, etc);
    add_child(vfs.root, tmp);
    add_child(vfs.root, home);
    add_child(vfs.root, dev);
    
    // Create some sample files
    VFSNode* readme = create_file("README.txt", FILE_TYPE_REGULAR);
    if(readme) {
        char* content = "Welcome to MyOS!\n\nThis is a simple operating system built from scratch.\n";
        readme->data = malloc(strlen(content) + 1);
        strcpy((char*)readme->data, content);
        readme->size = strlen(content);
        add_child(vfs.root, readme);
    }
    
    VFSNode* version = create_file("version.txt", FILE_TYPE_REGULAR);
    if(version) {
        char* content = "MyOS v1.0\nBuild: 2025-07-01\n";
        version->data = malloc(strlen(content) + 1);
        strcpy((char*)version->data, content);
        version->size = strlen(content);
        add_child(vfs.root, version);
    }
}

VFSNode* create_file(const char* name, unsigned int type) {
    if(vfs.node_count >= MAX_FILES) {
        return NULL;
    }
    
    VFSNode* node = &vfs.nodes[vfs.node_count++];
    strncpy(node->name, name, MAX_FILENAME_LENGTH - 1);
    node->name[MAX_FILENAME_LENGTH - 1] = '\0';
    node->type = type;
    node->permissions = PERM_READ | PERM_WRITE;
    node->size = 0;
    node->inode = vfs.node_count;
    node->parent = NULL;
    node->children = NULL;
    node->next = NULL;
    node->data = NULL;
    
    // Set default operations
    node->read = default_read;
    node->write = default_write;
    node->open = default_open;
    node->close = default_close;
    node->readdir = default_readdir;
    node->finddir = default_finddir;
    
    return node;
}

VFSNode* create_directory(const char* name) {
    return create_file(name, FILE_TYPE_DIRECTORY);
}

int add_child(VFSNode* parent, VFSNode* child) {
    if(!parent || !child || parent->type != FILE_TYPE_DIRECTORY) {
        return -1;
    }
    
    child->parent = parent;
    child->next = parent->children;
    parent->children = child;
    
    return 0;
}

VFSNode* find_file(const char* path) {
    if(!path || path[0] == '\0') {
        return NULL;
    }
    
    VFSNode* current = (path[0] == '/') ? vfs.root : vfs.current_dir;
    
    if(strcmp(path, "/") == 0) {
        return vfs.root;
    }
    
    char path_copy[MAX_PATH_LENGTH];
    strncpy(path_copy, path, MAX_PATH_LENGTH - 1);
    path_copy[MAX_PATH_LENGTH - 1] = '\0';
    
    // Skip leading slash
    char* token = strtok((path[0] == '/') ? path_copy + 1 : path_copy, "/");
    
    while(token && current) {
        if(strcmp(token, ".") == 0) {
            // Current directory - do nothing
        } else if(strcmp(token, "..") == 0) {
            // Parent directory
            if(current->parent) {
                current = current->parent;
            }
        } else {
            // Find child with matching name
            VFSNode* child = current->children;
            while(child) {
                if(strcmp(child->name, token) == 0) {
                    break;
                }
                child = child->next;
            }
            current = child;
        }
        token = strtok(NULL, "/");
    }
    
    return current;
}

int read_file(VFSNode* node, unsigned int offset, unsigned int size, char* buffer) {
    if(!node || !buffer || node->type != FILE_TYPE_REGULAR) {
        return -1;
    }
    
    return node->read(node, offset, size, buffer);
}

int write_file(VFSNode* node, unsigned int offset, unsigned int size, char* buffer) {
    if(!node || !buffer || node->type != FILE_TYPE_REGULAR) {
        return -1;
    }
    
    return node->write(node, offset, size, buffer);
}

int delete_file(const char* path) {
    VFSNode* node = find_file(path);
    if(!node || !node->parent) {
        return -1;
    }
    
    // Remove from parent's children list
    VFSNode* parent = node->parent;
    if(parent->children == node) {
        parent->children = node->next;
    } else {
        VFSNode* current = parent->children;
        while(current && current->next != node) {
            current = current->next;
        }
        if(current) {
            current->next = node->next;
        }
    }
    
    // Free data
    if(node->data) {
        free(node->data);
    }
    
    // Clear node
    node->name[0] = '\0';
    node->type = 0;
    node->size = 0;
    node->data = NULL;
    
    return 0;
}

char** list_directory(const char* path) {
    VFSNode* dir = find_file(path);
    if(!dir || dir->type != FILE_TYPE_DIRECTORY) {
        return NULL;
    }
    
    // Count children
    int count = 0;
    VFSNode* child = dir->children;
    while(child) {
        count++;
        child = child->next;
    }
    
    // Allocate array
    char** list = malloc((count + 1) * sizeof(char*));
    if(!list) {
        return NULL;
    }
    
    // Fill array
    child = dir->children;
    for(int i = 0; i < count; i++) {
        list[i] = malloc(strlen(child->name) + 1);
        strcpy(list[i], child->name);
        child = child->next;
    }
    list[count] = NULL;
    
    return list;
}

int change_directory(const char* path) {
    VFSNode* dir = find_file(path);
    if(!dir || dir->type != FILE_TYPE_DIRECTORY) {
        return -1;
    }
    
    vfs.current_dir = dir;
    return 0;
}

char* get_current_directory(void) {
    if(!vfs.current_dir) {
        return "/";
    }
    
    // Build path by traversing up to root
    char* path = malloc(MAX_PATH_LENGTH);
    path[0] = '\0';
    
    VFSNode* current = vfs.current_dir;
    char temp_path[MAX_PATH_LENGTH];
    temp_path[0] = '\0';
    
    while(current && current != vfs.root) {
        strcat(temp_path, "/");
        strcat(temp_path, current->name);
        current = current->parent;
    }
    
    if(temp_path[0] == '\0') {
        strcpy(path, "/");
    } else {
        // Reverse the path
        int len = strlen(temp_path);
        for(int i = 0; i < len; i++) {
            path[i] = temp_path[len - 1 - i];
        }
        path[len] = '\0';
    }
    
    return path;
}

// Default file operations
int default_read(VFSNode* node, unsigned int offset, unsigned int size, char* buffer) {
    if(!node->data || offset >= node->size) {
        return 0;
    }
    
    unsigned int bytes_to_read = (offset + size > node->size) ? 
                                 (node->size - offset) : size;
    
    memcpy(buffer, (char*)node->data + offset, bytes_to_read);
    return bytes_to_read;
}

int default_write(VFSNode* node, unsigned int offset, unsigned int size, char* buffer) {
    if(offset + size > node->size) {
        // Resize data buffer
        void* new_data = realloc(node->data, offset + size);
        if(!new_data) {
            return -1;
        }
        node->data = new_data;
        node->size = offset + size;
    }
    
    memcpy((char*)node->data + offset, buffer, size);
    return size;
}

int default_open(VFSNode* node) {
    return 0; // Success
}

int default_close(VFSNode* node) {
    return 0; // Success
}

VFSNode* default_readdir(VFSNode* node, unsigned int index) {
    if(node->type != FILE_TYPE_DIRECTORY) {
        return NULL;
    }
    
    VFSNode* child = node->children;
    for(unsigned int i = 0; i < index && child; i++) {
        child = child->next;
    }
    
    return child;
}

VFSNode* default_finddir(VFSNode* node, char* name) {
    if(node->type != FILE_TYPE_DIRECTORY) {
        return NULL;
    }
    
    VFSNode* child = node->children;
    while(child) {
        if(strcmp(child->name, name) == 0) {
            return child;
        }
        child = child->next;
    }
    
    return NULL;
}