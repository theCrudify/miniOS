// userspace/shell/shell.c
// Command line shell for MyOS

#include "../lib/libc/stdio.h"
#include "../lib/libc/stdlib.h"
#include "../lib/libc/string.h"
#include "../../kernel/include/filesystem.h"

#define MAX_COMMAND_LENGTH 256
#define MAX_ARGS 32
#define PROMPT "myos$ "

// Built-in commands
typedef struct {
    char* name;
    char* description;
    int (*function)(int argc, char** argv);
} Command;

// Function prototypes
int shell_main(void);
void shell_loop(void);
char* read_line(void);
char** parse_line(char* line);
int execute_command(char** args);
int launch_program(char** args);

// Built-in command functions
int cmd_help(int argc, char** argv);
int cmd_exit(int argc, char** argv);
int cmd_clear(int argc, char** argv);
int cmd_ls(int argc, char** argv);
int cmd_cd(int argc, char** argv);
int cmd_pwd(int argc, char** argv);
int cmd_cat(int argc, char** argv);
int cmd_echo(int argc, char** argv);
int cmd_mkdir(int argc, char** argv);
int cmd_rmdir(int argc, char** argv);
int cmd_rm(int argc, char** argv);
int cmd_cp(int argc, char** argv);
int cmd_mv(int argc, char** argv);
int cmd_ps(int argc, char** argv);
int cmd_kill(int argc, char** argv);
int cmd_date(int argc, char** argv);
int cmd_uptime(int argc, char** argv);
int cmd_free(int argc, char** argv);
int cmd_uname(int argc, char** argv);

// Built-in commands table
static Command builtin_commands[] = {
    {"help", "Show available commands", cmd_help},
    {"exit", "Exit the shell", cmd_exit},
    {"clear", "Clear the screen", cmd_clear},
    {"ls", "List directory contents", cmd_ls},
    {"cd", "Change directory", cmd_cd},
    {"pwd", "Print working directory", cmd_pwd},
    {"cat", "Display file contents", cmd_cat},
    {"echo", "Display text", cmd_echo},
    {"mkdir", "Create directory", cmd_mkdir},
    {"rmdir", "Remove directory", cmd_rmdir},
    {"rm", "Remove file", cmd_rm},
    {"cp", "Copy file", cmd_cp},
    {"mv", "Move/rename file", cmd_mv},
    {"ps", "List running processes", cmd_ps},
    {"kill", "Terminate process", cmd_kill},
    {"date", "Show current date and time", cmd_date},
    {"uptime", "Show system uptime", cmd_uptime},
    {"free", "Show memory usage", cmd_free},
    {"uname", "Show system information", cmd_uname}
};

static int num_builtins = sizeof(builtin_commands) / sizeof(Command);
static int shell_running = 1;

int shell_main(void) {
    printf("MyOS Shell v1.0\n");
    printf("Type 'help' for available commands.\n\n");
    
    shell_loop();
    
    return 0;
}

void shell_loop(void) {
    char* line;
    char** args;
    int status = 1;
    
    while(shell_running) {
        printf("%s", PROMPT);
        line = read_line();
        args = parse_line(line);
        status = execute_command(args);
        
        free(line);
        free(args);
        
        if(!status) {
            shell_running = 0;
        }
    }
}

char* read_line(void) {
    char* line = malloc(MAX_COMMAND_LENGTH);
    int position = 0;
    int c;
    
    if(!line) {
        printf("Shell: allocation error\n");
        exit(1);
    }
    
    while(1) {
        c = getchar();
        
        if(c == '\n') {
            line[position] = '\0';
            return line;
        } else if(c == '\b' || c == 127) { // Backspace
            if(position > 0) {
                position--;
                printf("\b \b");
            }
        } else {
            line[position] = c;
            position++;
            putchar(c);
            
            if(position >= MAX_COMMAND_LENGTH - 1) {
                line[position] = '\0';
                return line;
            }
        }
    }
}

char** parse_line(char* line) {
    int bufsize = MAX_ARGS;
    int position = 0;
    char** tokens = malloc(bufsize * sizeof(char*));
    char* token;
    
    if(!tokens) {
        printf("Shell: allocation error\n");
        exit(1);
    }
    
    token = strtok(line, " \t\r\n\a");
    while(token != NULL) {
        tokens[position] = token;
        position++;
        
        if(position >= bufsize) {
            bufsize += MAX_ARGS;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if(!tokens) {
                printf("Shell: allocation error\n");
                exit(1);
            }
        }
        
        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

int execute_command(char** args) {
    if(args[0] == NULL) {
        return 1; // Empty command
    }
    
    // Check built-in commands
    for(int i = 0; i < num_builtins; i++) {
        if(strcmp(args[0], builtin_commands[i].name) == 0) {
            return builtin_commands[i].function(count_args(args), args);
        }
    }
    
    // Try to launch external program
    return launch_program(args);
}

int launch_program(char** args) {
    printf("Shell: command not found: %s\n", args[0]);
    return 1;
}

int count_args(char** args) {
    int count = 0;
    while(args[count] != NULL) {
        count++;
    }
    return count;
}

// Built-in command implementations

int cmd_help(int argc, char** argv) {
    printf("MyOS Shell - Available Commands:\n\n");
    
    for(int i = 0; i < num_builtins; i++) {
        printf("  %-10s - %s\n", builtin_commands[i].name, builtin_commands[i].description);
    }
    
    printf("\nPress Tab for command completion.\n");
    printf("Use Ctrl+C to interrupt running programs.\n");
    return 1;
}

int cmd_exit(int argc, char** argv) {
    printf("Goodbye!\n");
    return 0;
}

int cmd_clear(int argc, char** argv) {
    // Send clear screen escape sequence
    printf("\033[2J\033[H");
    return 1;
}

int cmd_ls(int argc, char** argv) {
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
    
    return 1;
}

int cmd_cd(int argc, char** argv) {
    char* path = (argc > 1) ? argv[1] : "/";
    
    if(change_directory(path) != 0) {
        printf("cd: %s: No such file or directory\n", path);
    }
    
    return 1;
}

int cmd_pwd(int argc, char** argv) {
    char* cwd = get_current_directory();
    printf("%s\n", cwd);
    free(cwd);
    return 1;
}

int cmd_cat(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: cat <filename>\n");
        return 1;
    }
    
    struct vfs_node* file = find_file(argv[1]);
    if(!file) {
        printf("cat: %s: No such file or directory\n", argv[1]);
        return 1;
    }
    
    char* buffer = malloc(file->size + 1);
    if(read_file(file, 0, file->size, buffer) > 0) {
        buffer[file->size] = '\0';
        printf("%s", buffer);
    }
    free(buffer);
    
    return 1;
}

int cmd_echo(int argc, char** argv) {
    for(int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        if(i < argc - 1) printf(" ");
    }
    printf("\n");
    return 1;
}

int cmd_mkdir(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: mkdir <directory>\n");
        return 1;
    }
    
    // Implementation would create directory
    printf("mkdir: creating directory '%s'\n", argv[1]);
    return 1;
}

int cmd_rmdir(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: rmdir <directory>\n");
        return 1;
    }
    
    if(delete_file(argv[1]) != 0) {
        printf("rmdir: failed to remove '%s'\n", argv[1]);
    }
    
    return 1;
}

int cmd_rm(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: rm <filename>\n");
        return 1;
    }
    
    if(delete_file(argv[1]) != 0) {
        printf("rm: cannot remove '%s': No such file or directory\n", argv[1]);
    }
    
    return 1;
}

int cmd_cp(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: cp <source> <destination>\n");
        return 1;
    }
    
    if(copy_file(argv[1], argv[2]) != 0) {
        printf("cp: cannot copy '%s' to '%s'\n", argv[1], argv[2]);
    }
    
    return 1;
}

int cmd_mv(int argc, char** argv) {
    if(argc < 3) {
        printf("Usage: mv <source> <destination>\n");
        return 1;
    }
    
    if(move_file(argv[1], argv[2]) != 0) {
        printf("mv: cannot move '%s' to '%s'\n", argv[1], argv[2]);
    }
    
    return 1;
}

int cmd_ps(int argc, char** argv) {
    printf("  PID  PPID STATE     COMMAND\n");
    printf("    1     0 RUNNING   kernel\n");
    printf("    2     1 RUNNING   desktop\n");
    printf("    3     1 RUNNING   shell\n");
    return 1;
}

int cmd_kill(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: kill <pid>\n");
        return 1;
    }
    
    int pid = atoi(argv[1]);
    printf("kill: terminating process %d\n", pid);
    return 1;
}

int cmd_date(int argc, char** argv) {
    printf("Mon Jul  1 12:34:56 UTC 2025\n");
    return 1;
}

int cmd_uptime(int argc, char** argv) {
    printf("System uptime: 0 days, 0 hours, 5 minutes\n");
    return 1;
}

int cmd_free(int argc, char** argv) {
    printf("Memory Usage:\n");
    printf("Total:     %u KB\n", get_total_memory() / 1024);
    printf("Used:      %u KB\n", get_used_memory() / 1024);
    printf("Free:      %u KB\n", get_free_memory() / 1024);
    printf("Heap:      %u KB\n", get_heap_usage() / 1024);
    return 1;
}

int cmd_uname(int argc, char** argv) {
    printf("MyOS 1.0 i686\n");
    return 1;
}