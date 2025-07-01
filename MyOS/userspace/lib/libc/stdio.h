// userspace/lib/libc/stdio.h
#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>

typedef unsigned long size_t;

int printf(const char* format, ...);
int sprintf(char* str, const char* format, ...);
int vsprintf(char* str, const char* format, va_list args);
int putchar(int c);
int getchar(void);
int puts(const char* str);

#endif

// userspace/lib/libc/stdlib.h
#ifndef STDLIB_H
#define STDLIB_H

typedef unsigned long size_t;

void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void* calloc(size_t num, size_t size);

int atoi(const char* str);
char* itoa(int value, char* str, int base);
void exit(int status);
int abs(int x);
long labs(long x);
int rand(void);
void srand(unsigned int seed);

#endif

// userspace/lib/libc/string.h
#ifndef STRING_H
#define STRING_H

typedef unsigned long size_t;
#define SIZE_MAX (~(size_t)0)

size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);
int strcmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
char* strchr(const char* str, int c);
char* strrchr(const char* str, int c);
char* strstr(const char* haystack, const char* needle);
char* strtok(char* str, const char* delim);

void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memset(void* s, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
void* memchr(const void* s, int c, size_t n);

#endif

// userspace/lib/libc/math.h
#ifndef MATH_H
#define MATH_H

double sqrt(double x);
double pow(double base, double exp);
double sin(double x);
double cos(double x);

#define PI 3.14159265358979323846

#endif

// userspace/lib/libc/stdarg.h
#ifndef STDARG_H
#define STDARG_H

typedef char* va_list;

#define va_start(ap, last) (ap = (va_list)&last + sizeof(last))
#define va_arg(ap, type) (*(type*)((ap += sizeof(type)) - sizeof(type)))
#define va_end(ap) (ap = (va_list)0)

#endif