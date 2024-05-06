/* Inspired by @arget13/noexec.c and @jart/cosmopolitan/ape/loader.c */
/* ELF loader */

#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "../neatlibc/elf.h"

#ifdef __neatcc__       /* for later use in C library as internal execve() loader */
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#define Strlen                  strlen
#define StrCmp                  strcmp
#define MemMove                 memmove
#define Open                    open
#define Close                   close
#define Pread                   pread
#define Mmap                    mmap
#define Exit                    _exit
#define __builtin_memcpy(d,s,n) memcpy(d,s,n)
#define __builtin_va_list       va_list
#define __builtin_va_start      va_start
#define __builtin_va_arg        va_arg
#define __builtin_va_end        va_end
#define __builtin_unreachable()
//#include "/Users/greg/net/ncc/ldelf/syscalls.c"
#else
#include "syscalls.c"
#endif

#define debug       1
#define STACKADDR   ((void *)(0x080000000 - STACKLEN))  /* stack from 2G downwards */
#define STACKLEN    (2048 * 4096)                       /* 8MB stack */

extern void Launch(void *rdi, long entry, void *sp, int rcx) __attribute__((__noreturn__));

static Elf64_Addr search_section(int fd, char* section)
{
    uint16_t shnum;
    uint16_t shstrndx;
    int i;
    char* shstrtab;
    Elf64_Shdr* shdr;
    Elf64_Ehdr ehdr;

    Pread(fd, &ehdr, sizeof(ehdr), 0);

    shnum = ehdr.e_shnum;
    shdr = alloca(sizeof(*shdr) * shnum);
    Pread(fd, shdr, sizeof(*shdr) * shnum, ehdr.e_shoff);

    shstrndx = ehdr.e_shstrndx;
    shstrtab = alloca(shdr[shstrndx].sh_size);
    Pread(fd, shstrtab, shdr[shstrndx].sh_size, shdr[shstrndx].sh_offset);

    for(i = 0; i < shnum; ++i)
        if(!StrCmp(&shstrtab[shdr[i].sh_name], section))
        {
            return shdr[i].sh_addr;
        }

    return 0;
}

/* declaring this function static causes call ___bzero */
Elf64_Ehdr *load(char* path, Elf64_Addr rebase)
{
    int fd, i;
    Elf64_Phdr* phdr;
    uint16_t phnum;
    Elf64_Addr bss;
    long rc;
    static Elf64_Ehdr ehdr;

    if((fd = Open(path, O_RDONLY, 0)) < 0)
    {
        Print(2, "Can't open ", path, "\n", 0);
        Exit(0);
    }
    Pread(fd, &ehdr, sizeof(ehdr), 0);
    phnum = ehdr.e_phnum;
    phdr = alloca(sizeof(*phdr) * phnum);
    Pread(fd, phdr, sizeof(*phdr) * phnum, ehdr.e_phoff);
    bss = search_section(fd, ".bss");
    DEBUG1("phnum       ", phnum);
    DEBUG1("bss section ", bss);

    for(i = 0; i < phnum; ++i)
    {
        if(phdr[i].p_type != PT_LOAD) continue;

        uint32_t   flags   = phdr[i].p_flags;
        Elf64_Off  offset  = phdr[i].p_offset;
        Elf64_Addr vaddr   = phdr[i].p_vaddr;
        size_t     filesz  = phdr[i].p_filesz;
        size_t     memsz   = phdr[i].p_memsz;
        char *     aligned = (char *) (vaddr & (~0xfff));

        uint32_t prot = ((flags & PF_R) ? PROT_READ  : 0) |
                        ((flags & PF_W) ? PROT_WRITE : 0) |
                        ((flags & PF_X) ? PROT_EXEC  : 0);

        filesz += vaddr - (Elf64_Addr) aligned;
        memsz  += vaddr - (Elf64_Addr) aligned;
        offset -= vaddr - (Elf64_Addr) aligned;
        size_t _filesz = (filesz + 0xfff) & ~0xfff;

        //if (filesz == 0) continue;
        rc = Mmap(rebase + aligned, filesz, prot, MAP_PRIVATE | MAP_FIXED, fd, offset);
        DEBUG1("load mmap   ", rc);
        if(memsz > _filesz)
        {
            void* extra = rebase + aligned + _filesz;
            rc = Mmap(extra, memsz - _filesz, prot, MAP_PRIVATE | MAP_FIXED | MAP_ANON, -1, 0);
            DEBUG1("zero mmap   ", rc);
            DEBUG1("     size   ", memsz - _filesz);
        }

        if(bss != 0 && (bss >= vaddr && bss < (vaddr + filesz)))
        {
            size_t bss_size = _filesz - (bss - (Elf64_Addr) aligned);
            Bzero((char *)rebase + bss, bss_size);
            DEBUG1("zero bss    ", rebase + bss);
            DEBUG1("     size   ", bss_size);
        }
    }
    Close(fd);
    return &ehdr;
}

__attribute__((noreturn))
static void run(int argc, char** argv, int readfd, int writefd)
{
    (void)readfd;
    (void)writefd;

    Elf64_Ehdr *ehdr = load(argv[0], 0);
    char *stack = (char *)Mmap(STACKADDR, STACKLEN, PROT_READ | PROT_WRITE,
                 MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    void **sp = (void**) &stack[STACKLEN];
    uint64_t entry = ehdr->e_entry;
    DEBUG1("entry       ", entry);
    DEBUG1("stack start ", (long)sp);
    DEBUG1("stack end   ", (long)stack);

#if 0       /* create auxv, copy but don't rewrite argv/envp addresses */
    char** envp;
    int envc;
    Elf64_Addr base = 0x400000;     // FIXME may vary
    uint64_t phnum     = ehdr->e_phnum;
    uint64_t phentsize = ehdr->e_phentsize;
    uint64_t phaddr    = ehdr->e_phoff + base;
    uint64_t auxv[8 * 2];

    *--sp = NULL; // End of stack
    if(argc & 1)
        *--sp = NULL; // Keep stack aligned
    auxv[ 0] = 0x06; auxv[ 1] = 0x1000;    // AT_PAGESZ
    auxv[ 2] = 0x19; auxv[ 3] = 0;   	   // AT_RANDOM (whatever)
    auxv[ 4] = 0x09; auxv[ 5] = entry;     // AT_ENTRY
    auxv[ 6] = 0x07; auxv[ 7] = base;      // AT_BASE
    auxv[ 8] = 0x05; auxv[ 9] = phnum;     // AT_PHNUM
    auxv[10] = 0x04; auxv[11] = phentsize; // AT_PHENT
    auxv[12] = 0x03; auxv[13] = phaddr;    // AT_PHDR
    auxv[14] =    0; auxv[15] = 0;         // End of auxv
    sp -= sizeof(auxv) / sizeof(*auxv); MemMove(sp, auxv, sizeof(auxv));

    *--sp = NULL; // End of envp
    envp = (char **)(argv + argc + 1);
    for (envc = 0; *envp++; envc++);
    if (envc) {
        envp = (char **)(argv + argc + 1);
        sp -= envc; MemMove(sp, envp, envc * 8);
    }

    *--sp = NULL; // End of argv
    sp -= argc; MemMove(sp, argv, argc * 8);
    *(size_t*) --sp = argc;
#else        /* copy argv/envp addresses into 32-bit address space */
    int n, bytes = 0;
    uint64_t *lp = (uint64_t *)argv;      /* skip argc */
    while (*lp)
        bytes += StrLen((char *)*lp++) + 1;
    lp++;                         /* end argv */
    while (*lp)
        bytes += StrLen((char *)*lp++) + 1;
    lp += 2;                      /* end envp, null auxv */
    bytes += (char *)lp - (char *)argv + sizeof(uint64_t);
    bytes = (bytes + 15) & ~15;
    char *p = (char *)(stack + STACKLEN);
    uint64_t *sp2 = (void *)(stack + STACKLEN - bytes);

    *sp2++ = argc;               /* argc */
    while (*argv) {
        n = StrLen(*argv) + 1;
        p -= n;
        MemMove(p, *argv, n);
        *sp2++ = (uint64_t)p;
        argv++;
    }
    *sp2++ = (uint64_t)*argv++;  /* argv null */

    while (*argv) {
        n = StrLen(*argv) + 1;
        p -= n;
        MemMove(p, *argv, n);
        *sp2++ = (uint64_t)p;
        argv++;
    }
    *sp2++ = 0;                   /* envp null */
    *sp2++ = 0;                   /* null aux */
    sp = (void *)(stack + STACKLEN - bytes);
    DEBUG1("ARGS  ", bytes);
#endif

#if 0
    if(readfd >= 0)
    {
        dup2(readfd, STDIN_FILENO);
        close(readfd);
    }
    if(writefd >= 0)
    {
        dup2(writefd, STDOUT_FILENO);
        close(writefd);
    }
#endif

    DEBUG1("launch      ", entry);
    Launch(0, entry, sp, 0);
    __builtin_unreachable();
}

__attribute__((__noreturn__)) void Main(long di, long *sp, char dl)
{
    int argc;
    char **argv;

    sp = InitAPE(di, sp, dl);
    argc = *sp;
    argv = (char **)(sp + 1);

    if (argc > 1)
        run(argc-1, argv+1, 0, 1);
    else Print(2, "Usage: load <program> [args...]\n", 0);
    Exit(1);
}
