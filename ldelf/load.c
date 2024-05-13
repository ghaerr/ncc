/*
 * ELF loader compilable by ncc, gcc and clang for macOS and Linux.
 *
 * This loader is compiled and used by the host OS to initially load
 * NCC ELF binaries, as well as compiled by NCC and used within NCC
 * binaries as an exec function.
 *
 * Inspired by @arget13/noexec.c and @jart/cosmopolitan/ape/loader.c
 *
 * May 2024 Greg Haerr
 */

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "../neatlibc/elf.h"
#include "../neatlibc/syscalls.h"

#define debug       1

#ifdef __neatcc__       /* for use in C library as internal execve() loader */
#include <assert.h>
/* ncc requires both sides of token paste to have no space and be macro parms! */
#define alloc(p,n,sz)           char p##n [65536]; p = (void *)p##n
#define STACKTOP    0x70000000          /* stack from 2G-256M downwards */
#define STACKLEN    0x00800000          /* 8MB stack */
#define BASEADDR    0x00400000          /* lowest loadable program segment */
#else
#define STACKTOP    0x80000000          /* stack from 2G downwards */
#define STACKLEN    0x00800000          /* 8MB stack */
#define alloc(p,n,sz)           p = alloca(sz)
#define unassert(expr)
#endif

#define DEBUG(STR)                              \
  if (debug) {                                  \
    Print(2, STR, 0);                           \
  }

#define DEBUG1(STR,VAR)                         \
  if (debug) {                                  \
    char ibuf[32];                              \
    Print(2, STR, HexStr(ibuf, VAR), "\n", 0);  \
  }

static char *HexStr(char *buf, unsigned long x) {
  char *p = buf;
  int i = 64;
  if (x) {
    do {
      if (i == 32) *p++ = '_';
      *p++ = "0123456789abcdef"[(x >> (i -= 4)) & 15];
    } while (i);
  } else {
    *p++ = '0';
  }
  *p = 0;
  return buf;
}

static long Print(int fd, const char *s, ...) {
  int c;
  unsigned n;
  char b[512];
  __builtin_va_list va;
  __builtin_va_start(va, s);
  for (n = 0; s; s = __builtin_va_arg(va, const char *)) {
    while ((c = *s++)) {
      if (n < sizeof(b)) {
        b[n++] = c;
      }
    }
  }
  __builtin_va_end(va);
  return Write(fd, b, n);
}

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
    alloc(shdr, 1, sizeof(*shdr) * shnum);
    Pread(fd, shdr, sizeof(*shdr) * shnum, ehdr.e_shoff);

    shstrndx = ehdr.e_shstrndx;
    alloc(shstrtab, 2, shdr[shstrndx].sh_size);
    Pread(fd, shstrtab, shdr[shstrndx].sh_size, shdr[shstrndx].sh_offset);

    for(i = 0; i < shnum; ++i)
        if(!StrCmp(&shstrtab[shdr[i].sh_name], section))
        {
            return shdr[i].sh_addr;
        }

    return 0;
}

/* declaring this function static causes call ___bzero */
Elf64_Ehdr *Load(char* path, Elf64_Addr rebase)
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
    if (ehdr.e_ident[0] != 0x7f && ehdr.e_ident[1] != 'E') {
        Print(2, path, ": not ELF\n", 0);
        Close(fd);
        return 0;
    }

    phnum = ehdr.e_phnum;
    alloc(phdr, 1, sizeof(*phdr) * phnum);
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

#ifdef __neatcc__
        if (aligned < BASEADDR) {
            Print("Bad load address: ", aligned, "\n", 0);
            Close(fd);
            return 0;
        }
#endif

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

static void run(int argc, char** argv, int readfd, int writefd)
{
    Elf64_Ehdr *ehdr;
    (void)readfd;
    (void)writefd;

    if (!(ehdr = Load(argv[0], 0)))
        return;
    char *stack = (char *)Mmap((void *)(STACKTOP - STACKLEN), STACKLEN,
        PROT_READ | PROT_WRITE, MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    unassert(stack == (void *)(STACKTOP - STACKLEN));

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
    auxv[ 2] = 0x19; auxv[ 3] = 0;         // AT_RANDOM (whatever)
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
    DEBUG1("args size   ", bytes);
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

#define MAXARGS     40
#define CMDLEN      80
#define isblank(c)	((c) == ' ' || (c) == '\t')

static int makeargs(char *cmd, int *argcptr, char ***argvptr)
{
    char    *cp;
    int     argc;
    static char strings[CMDLEN+1];
    static char *argtable[MAXARGS+1];

    StrCpy(strings, cmd);
    argc = 0;
    cp = strings;
    while (*cp) {
        if (argc >= MAXARGS)
            return 0;
        argtable[argc++] = cp;
        while (*cp && !isblank(*cp))
            cp++;
        while (isblank(*cp))
            *cp++ = '\0';
    }
    argtable[argc] = NULL;
    *argcptr = argc;
    *argvptr = argtable;
    return 1;
}

#ifdef __neatcc__
int main(int argc, char **argv) {
#else
__attribute__((__noreturn__)) void Main(long di, long *sp, char dl) {
    sp = InitAPE(di, sp, dl);
    int argc = *sp;
    char **argv = (char **)(sp + 1);
#endif
    if (argc > 1) {
        run(argc-1, argv+1, 0, 1);
    } else for (;;) {       /* interactive shell mode */
        long rc;
        int ac;
        char **av;
        char buf[80];

        Print(2, "% ", 0);
        rc = Read(0, buf, 80);
        if (rc <= 0) Exit(0);
        buf[rc - 1] = '\0';
        if (!buf[0])
            continue;
        if (Fork() == 0) {
            if (makeargs(buf, &ac, &av))
                run(ac, av, 0, 1);
            Print(2, "Load failed: ", av[0], "\n", 0);
            Exit(255);
        }
        int status;
        WaitPid(-1, &status, 0);
        DEBUG1("exit status ", WEXITSTATUS(status));
    }
    Exit(1);
}
