// userspace/lib/libc/stdio.c
// Standard I/O library

#include "stdio.h"
#include "string.h"
#include "../../../kernel/include/kernel.h"

// System call numbers
#define SYS_WRITE 1
#define SYS_READ 2

int printf(const char* format, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, format);
    int result = vsprintf(buffer, format, args);
    va_end(args);
    
    // Write to console
    syscall(SYS_WRITE, 1, (int)buffer, strlen(buffer));
    return result;
}

int sprintf(char* str, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsprintf(str, format, args);
    va_end(args);
    return result;
}

int vsprintf(char* str, const char* format, va_list args) {
    char* ptr = str;
    const char* fmt = format;
    
    while(*fmt) {
        if(*fmt == '%') {
            fmt++;
            switch(*fmt) {
                case 'd': {
                    int val = va_arg(args, int);
                    char num_str[32];
                    itoa(val, num_str, 10);
                    strcpy(ptr, num_str);
                    ptr += strlen(num_str);
                    break;
                }
                case 'x': {
                    unsigned int val = va_arg(args, unsigned int);
                    char num_str[32];
                    itoa(val, num_str, 16);
                    strcpy(ptr, num_str);
                    ptr += strlen(num_str);
                    break;
                }
                case 's': {
                    char* val = va_arg(args, char*);
                    strcpy(ptr, val);
                    ptr += strlen(val);
                    break;
                }
                case 'c': {
                    char val = va_arg(args, int);
                    *ptr++ = val;
                    break;
                }
                case '%': {
                    *ptr++ = '%';
                    break;
                }
                default:
                    *ptr++ = '%';
                    *ptr++ = *fmt;
                    break;
            }
        } else {
            *ptr++ = *fmt;
        }
        fmt++;
    }
    *ptr = '\0';
    return ptr - str;
}

int putchar(int c) {
    char ch = c;
    syscall(SYS_WRITE, 1, (int)&ch, 1);
    return c;
}

int getchar(void) {
    char ch;
    syscall(SYS_READ, 0, (int)&ch, 1);
    return ch;
}

int puts(const char* str) {
    printf("%s\n", str);
    return strlen(str) + 1;
}

// userspace/lib/libc/stdlib.c
// Standard library functions

#include "stdlib.h"
#include "string.h"
#include "../../../kernel/include/memory.h"

void* malloc(size_t size) {
    return kmalloc(size);
}

void free(void* ptr) {
    kfree(ptr);
}

void* realloc(void* ptr, size_t size) {
    if(!ptr) return malloc(size);
    if(!size) {
        free(ptr);
        return NULL;
    }
    
    void* new_ptr = malloc(size);
    if(new_ptr) {
        // Copy old data (simplified - doesn't know original size)
        memcpy(new_ptr, ptr, size);
        free(ptr);
    }
    return new_ptr;
}

void* calloc(size_t num, size_t size) {
    size_t total = num * size;
    void* ptr = malloc(total);
    if(ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

int atoi(const char* str) {
    int result = 0;
    int sign = 1;
    
    // Skip whitespace
    while(*str == ' ' || *str == '\t') str++;
    
    // Handle sign
    if(*str == '-') {
        sign = -1;
        str++;
    } else if(*str == '+') {
        str++;
    }
    
    // Convert digits
    while(*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

char* itoa(int value, char* str, int base) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    int tmp_value;
    
    // Handle negative numbers for base 10
    if(value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
        ptr1++;
    }
    
    // Convert to string (reversed)
    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789abcdef"[tmp_value - value * base];
    } while(value);
    
    *ptr-- = '\0';
    
    // Reverse string
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
    
    return str;
}

void exit(int status) {
    // Terminate current process
    while(1) {
        asm volatile("hlt");
    }
}

int abs(int x) {
    return x < 0 ? -x : x;
}

long labs(long x) {
    return x < 0 ? -x : x;
}

// userspace/lib/libc/string.c
// String manipulation functions

#include "string.h"

size_t strlen(const char* str) {
    size_t len = 0;
    while(str[len]) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* orig_dest = dest;
    while((*dest++ = *src++));
    return orig_dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    char* orig_dest = dest;
    while(n-- && (*dest++ = *src++));
    while(n-- > 0) *dest++ = '\0';
    return orig_dest;
}

char* strcat(char* dest, const char* src) {
    char* orig_dest = dest;
    while(*dest) dest++;
    while((*dest++ = *src++));
    return orig_dest;
}

char* strncat(char* dest, const char* src, size_t n) {
    char* orig_dest = dest;
    while(*dest) dest++;
    while(n-- && (*dest++ = *src++));
    *dest = '\0';
    return orig_dest;
}

int strcmp(const char* str1, const char* str2) {
    while(*str1 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

int strncmp(const char* str1, const char* str2, size_t n) {
    while(n-- && *str1 && *str1 == *str2) {
        str1++;
        str2++;
    }
    return n == SIZE_MAX ? 0 : *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strchr(const char* str, int c) {
    while(*str) {
        if(*str == c) return (char*)str;
        str++;
    }
    return c == '\0' ? (char*)str : NULL;
}

char* strrchr(const char* str, int c) {
    char* last = NULL;
    while(*str) {
        if(*str == c) last = (char*)str;
        str++;
    }
    return c == '\0' ? (char*)str : last;
}

char* strstr(const char* haystack, const char* needle) {
    if(!*needle) return (char*)haystack;
    
    while(*haystack) {
        const char* h = haystack;
        const char* n = needle;
        
        while(*h && *n && *h == *n) {
            h++;
            n++;
        }
        
        if(!*n) return (char*)haystack;
        haystack++;
    }
    
    return NULL;
}

char* strtok(char* str, const char* delim) {
    static char* next_token = NULL;
    
    if(str) next_token = str;
    if(!next_token) return NULL;
    
    // Skip leading delimiters
    while(*next_token && strchr(delim, *next_token)) {
        next_token++;
    }
    
    if(!*next_token) {
        next_token = NULL;
        return NULL;
    }
    
    char* token_start = next_token;
    
    // Find end of token
    while(*next_token && !strchr(delim, *next_token)) {
        next_token++;
    }
    
    if(*next_token) {
        *next_token++ = '\0';
    } else {
        next_token = NULL;
    }
    
    return token_start;
}

void* memcpy(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    while(n--) {
        *d++ = *s++;
    }
    
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    char* d = (char*)dest;
    const char* s = (const char*)src;
    
    if(d < s) {
        while(n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while(n--) *--d = *--s;
    }
    
    return dest;
}

void* memset(void* s, int c, size_t n) {
    char* ptr = (char*)s;
    while(n--) {
        *ptr++ = c;
    }
    return s;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    
    while(n--) {
        if(*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    
    return 0;
}

void* memchr(const void* s, int c, size_t n) {
    const unsigned char* ptr = (const unsigned char*)s;
    
    while(n--) {
        if(*ptr == (unsigned char)c) {
            return (void*)ptr;
        }
        ptr++;
    }
    
    return NULL;
}

// userspace/lib/libc/math.c
// Basic math functions

#include "math.h"

double sqrt(double x) {
    if(x < 0) return -1; // Error
    if(x == 0) return 0;
    
    double guess = x / 2.0;
    double prev_guess;
    
    do {
        prev_guess = guess;
        guess = (guess + x / guess) / 2.0;
    } while(abs(guess - prev_guess) > 0.0001);
    
    return guess;
}

double pow(double base, double exp) {
    if(exp == 0) return 1;
    if(exp == 1) return base;
    if(exp < 0) return 1.0 / pow(base, -exp);
    
    double result = 1;
    while(exp > 0) {
        if((int)exp % 2 == 1) {
            result *= base;
        }
        base *= base;
        exp /= 2;
    }
    
    return result;
}

double sin(double x) {
    // Taylor series approximation
    double result = 0;
    double term = x;
    int sign = 1;
    
    for(int i = 0; i < 10; i++) {
        result += sign * term;
        term *= x * x / ((2 * i + 2) * (2 * i + 3));
        sign *= -1;
    }
    
    return result;
}

double cos(double x) {
    // Taylor series approximation
    double result = 1;
    double term = 1;
    int sign = -1;
    
    for(int i = 1; i < 10; i++) {
        term *= x * x / ((2 * i - 1) * (2 * i));
        result += sign * term;
        sign *= -1;
    }
    
    return result;
}

int rand(void) {
    static unsigned int seed = 1;
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

void srand(unsigned int new_seed) {
    static unsigned int* seed_ptr = NULL;
    if(!seed_ptr) {
        static unsigned int seed = 1;
        seed_ptr = &seed;
    }
    *seed_ptr = new_seed;
}