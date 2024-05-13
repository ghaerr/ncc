/* system call wrappers for static NCC and host binaries */

#ifdef __neatcc__
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#define StrLen                  strlen
#define StrCmp                  strcmp
#define StrCpy                  strcpy
#define MemMove                 memmove
#define Bzero(d,n)              memset(d,0,n)
#define Open                    open
#define Close                   close
#define Pread                   pread
#define Read                    read
#define Write                   write
#define Mmap                    mmap
#define WaitPid                 waitpid
#define Fork                    fork
#define Exit                    _exit
#define __builtin_memcpy(d,s,n) memcpy(d,s,n)
#define __builtin_va_list       va_list
#define __builtin_va_start      va_start
#define __builtin_va_arg        va_arg
#define __builtin_va_end        va_end
#define __builtin_unreachable()

#else
long *InitAPE(long di, long *sp, char dl);
__attribute__((__noreturn__)) void Exit(int rc);
long StrLen(const char *s);
char *StrCpy(char *d, const char *s);
int StrCmp(const char *l, const char *r);
void Bzero(void *a, unsigned long n);
void *MemMove(void *a, const void *b, unsigned long n);
int Open(const char *path, int flags, int mode);
int Close(int fd);
long Read(int fd, void *data, unsigned long size);
long Write(int fd, const void *data, unsigned long size);
long Pread(int fd, void *data, unsigned long size, long off);
long Mmap(void *addr, unsigned long size, int prot, int flags, int fd, long off);
long Fork(void);
long WaitPid(int pid, int *status, int options);
#endif

void Launch(void *rdi, long entry, void *sp, int rcx) __attribute__((__noreturn__));
