/* Inspired by @arget13/noexec.c and @jart/cosmopolitan/ape/loader.c */
/* ELF loader */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <libgen.h>
#include <signal.h>
#include <termios.h>
#include "../neatlibc/elf.h"

#define debug       1
#define STACKADDR   ((void *)(0x080000000 - STACKLEN))  /* stack from 2G downwards */
#define STACKLEN    (2048 * 4096)                       /* 8MB stack */

Elf64_Addr search_section(int fd, char* section);

#define SUPPORT_VECTOR      XNU
#define LINUX   1
#define XNU     8
#define OPENBSD 16
#define FREEBSD 32
#define NETBSD  64

#define SupportsLinux()   (SUPPORT_VECTOR & LINUX)
#define SupportsXnu()     (SUPPORT_VECTOR & XNU)
#define SupportsFreebsd() (SUPPORT_VECTOR & FREEBSD)
#define SupportsOpenbsd() (SUPPORT_VECTOR & OPENBSD)
#define SupportsNetbsd()  (SUPPORT_VECTOR & NETBSD)

#define IsLinux()   (SupportsLinux() && os == LINUX)
#define IsXnu()     (SupportsXnu() && os == XNU)
#define IsFreebsd() (SupportsFreebsd() && os == FREEBSD)
#define IsOpenbsd() (SupportsOpenbsd() && os == OPENBSD)
#define IsNetbsd()  (SupportsNetbsd() && os == NETBSD)

#ifdef __aarch64__
#define IsAarch64() 1
#else
#define IsAarch64() 0
#endif

#define DEBUG(STR)                          \
  if (debug) {                              \
    Print(XNU, 2, STR, 0l);                  \
  }

#define DEBUG1(STR,VAR)                     \
  if (debug) {                              \
    char ibuf[32];                          \
    Utox(ibuf, VAR);                        \
    Print(XNU, 2, STR, ibuf, "\n", 0l);      \
  }

void Launch(void *rdi, long entry, void *sp, int rcx) __attribute__((__noreturn__));
long SystemCall(long, long, long, long, long, long, long, int);

__attribute__((__noinline__)) static long CallSystem(long arg1, long arg2,
                                                     long arg3, long arg4,
                                                     long arg5, long arg6,
                                                     long arg7, int numba,
                                                     char os) {
  if (IsXnu()) numba |= 0x2000000;
  return SystemCall(arg1, arg2, arg3, arg4, arg5, arg6, arg7, numba);
}

static long Write(int fd, const void *data, unsigned long size, int os) {
  int numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 64;
    } else {
      numba = 1;
    }
  } else {
    numba = 4;
  }
  return CallSystem(fd, (long)data, size, 0, 0, 0, 0, numba, os);
}

static long Print(int os, int fd, const char *s, ...) {
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
  return Write(fd, b, n, os);
}

static char *Utox(char p[19], unsigned long x) {
  int i;
  if (x) {
#if 0   /* hex formatting */
    *p++ = '0';
    *p++ = 'x';
    i = (__builtin_clzl(x) ^ (sizeof(long) * 8 - 1)) + 1;
    i = (i + 3) & -4;
#else
    i = 64;
#endif
    do {
      if (i == 32) *p++ = '_';
      *p++ = "0123456789abcdef"[(x >> (i -= 4)) & 15];
    } while (i);
  } else {
    *p++ = '0';
  }
  *p = 0;
  return p;
}

__attribute__((__noreturn__)) static void Exit(long rc, int os) {
  int numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 94;
    } else {
      numba = 60;
    }
  } else {
    numba = 1;
  }
  CallSystem(rc, 0, 0, 0, 0, 0, 0, numba, os);
  __builtin_unreachable();
}

static int StrCmp(const char *l, const char *r) {
  unsigned long i = 0;
  while (l[i] == r[i] && r[i]) ++i;
  return (l[i] & 255) - (r[i] & 255);
}

static void Bzero(void *a, unsigned long n) {
  long z;
  volatile char *p;
  char *e;
  p = (char *)a;
  e = (char *)p + n;
  z = 0;
  while (p + sizeof(z) <= e) {
    __builtin_memcpy((void *)p, &z, sizeof(z));
    p += sizeof(z);
  }
  while (p < e) {   /* if not volatile, ___bzero called here on -Os */
    *p++ = 0;
  }
}

static void *MemMove(void *a, const void *b, unsigned long n) {
  long w;
  char *d;
  const char *s;
  unsigned long i;
  d = (char *)a;
  s = (const char *)b;
  if (d > s) {
    while (n >= sizeof(w)) {
      n -= sizeof(w);
      __builtin_memcpy(&w, s + n, sizeof(n));
      __builtin_memcpy(d + n, &w, sizeof(n));
    }
    while (n--) {
      d[n] = s[n];
    }
  } else {
    i = 0;
    while (i + sizeof(w) <= n) {
      __builtin_memcpy(&w, s + i, sizeof(i));
      __builtin_memcpy(d + i, &w, sizeof(i));
      i += sizeof(w);
    }
    for (; i < n; ++i) {
      d[i] = s[i];
    }
  }
  return d;
}

static long Pread(int fd, void *data, unsigned long size, long off, int os) {
  long numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 0x043;
    } else {
      numba = 0x011;
    }
  } else if (IsXnu()) {
    numba = 0x2000099;
  } else if (IsFreebsd()) {
    numba = 0x1db;
  } else if (IsOpenbsd()) {
    numba = 0x0a9; /* OpenBSD v7.3+ */
  } else if (IsNetbsd()) {
    numba = 0x0ad;
  } else {
    __builtin_unreachable();
  }
  return SystemCall(fd, (long)data, size, off, off, 0, 0, numba);
}

static int Open(const char *path, int flags, int mode, int os) {
  if (IsLinux() && IsAarch64()) {
    return SystemCall(-100, (long)path, flags, mode, 0, 0, 0, 56);
  } else {
    return CallSystem((long)path, flags, mode, 0, 0, 0, 0, IsLinux() ? 2 : 5,
                      os);
  }
}

static int Close(int fd, int os) {
  int numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 57;
    } else {
      numba = 3;
    }
  } else {
    numba = 6;
  }
  return CallSystem(fd, 0, 0, 0, 0, 0, 0, numba, os);
}

static long Mmap(void *addr, unsigned long size, int prot, int flags, int fd,
                 long off, int os) {
  long numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 222;
    } else {
      numba = 9;
    }
  } else if (IsXnu()) {
    numba = 0x2000000 | 197;
  } else if (IsFreebsd()) {
    numba = 477;
  } else if (IsOpenbsd()) {
    numba = 49; /* OpenBSD v7.3+ */
  } else if (IsNetbsd()) {
    numba = 197;
  } else {
    __builtin_unreachable();
  }
  return SystemCall((long)addr, size, prot, flags, fd, off, off, numba);
}


Elf64_Ehdr *load(char* path, Elf64_Addr rebase)
{
    int fd;
    Elf64_Phdr* phdr;
    uint16_t phnum;
    Elf64_Addr bss;
	long rc;
    static Elf64_Ehdr ehdr;

    if((fd = Open(path, O_RDONLY, 0, XNU)) < 0)
    {
        Print(XNU, 2, "Can't open ", path, "\n", 0);
        Exit(0, XNU);
    }
    Pread(fd, &ehdr, sizeof(ehdr), 0, XNU);
    phnum = ehdr.e_phnum;
    phdr = alloca(sizeof(*phdr) * phnum);
    Pread(fd, phdr, sizeof(*phdr) * phnum, ehdr.e_phoff, XNU);
    bss = search_section(fd, ".bss");
	DEBUG1("phnum       ", phnum);
	DEBUG1("bss section ", bss);

    for(int i = 0; i < phnum; ++i)
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
        rc = Mmap(rebase + aligned, filesz, prot, MAP_PRIVATE | MAP_FIXED, fd, offset, XNU);
		DEBUG1("load mmap   ", rc);
        if(memsz > _filesz)
        {
            void* extra = rebase + aligned + _filesz;
            rc = Mmap(extra, memsz - _filesz, prot, MAP_PRIVATE | MAP_FIXED | MAP_ANON, -1, 0, XNU);
			DEBUG1("zero mmap   ", rc);
			DEBUG1("     size   ", memsz - _filesz)
        }

        if(bss != 0 && (bss >= vaddr && bss < (vaddr + filesz)))
        {
            size_t bss_size = _filesz - (bss - (Elf64_Addr) aligned);
            Bzero((char *)rebase + bss, bss_size);
			DEBUG1("zero bss    ", rebase + bss);
			DEBUG1("     size   ", bss_size);
        }
    }
    Close(fd, XNU);
	return &ehdr;
}

Elf64_Addr search_section(int fd, char* section)
{
    Elf64_Ehdr ehdr;
    Elf64_Shdr* shdr;
    uint16_t shnum;
    uint16_t shstrndx;
    char* shstrtab;

    Pread(fd, &ehdr, sizeof(ehdr), 0, XNU);

    shnum = ehdr.e_shnum;
    shdr = alloca(sizeof(*shdr) * shnum);
    Pread(fd, shdr, sizeof(*shdr) * shnum, ehdr.e_shoff, XNU);

    shstrndx = ehdr.e_shstrndx;
    shstrtab = alloca(shdr[shstrndx].sh_size);
    Pread(fd, shstrtab, shdr[shstrndx].sh_size, shdr[shstrndx].sh_offset, XNU);

    for(int i = 0; i < shnum; ++i)
        if(!StrCmp(&shstrtab[shdr[i].sh_name], section))
        {
            return shdr[i].sh_addr;
        }

    return 0;
}

#ifdef __linux__
void* ld_addr()
{
    FILE* f = fopen("/proc/self/maps", "rb");
    char buf[1024];
    void* p;
    while(fgets(buf, sizeof buf, f))
    {
        if(strncmp(basename(strchr(buf, '/')), "ld", 2)) continue;
        sscanf(buf, "%lx", &p);
        fclose(f);
        return p;
    }
    fclose(f);
    return NULL;
}

char* search_path(char* cmd)
{
    if(*cmd == '/') return cmd;
    char* dup, * path, * p;
    char* filepath;
    dup = path = p = strdup(getenv("PATH"));
    struct stat buf;
    do
    {
        p = strchr(p, ':');
        if(p != NULL)
            *p++ = '\0';

        filepath = malloc(strlen(path) + strlen(cmd) + 1 + 1);
        strcpy(filepath, path);
        strcat(filepath, "/");
        strcat(filepath, cmd);
        if(fstatat(AT_FDCWD, filepath, &buf, 0) == 0)
        {
            free(dup);
            return filepath;
        }

        free(filepath);
        path = p;
    }
    while(p != NULL);

    free(dup);
    return NULL;
}
#endif

__attribute__((noreturn))
void run(int argc, char** argv, int readfd, int writefd)
{
    Elf64_Addr base = 0x400000;
    Elf64_Ehdr *ehdr;
    uint64_t entry, phnum, phentsize, phaddr;
    uint64_t auxv[8 * 2];
    char* stack;
    void** sp;

    (void)readfd;
    (void)writefd;
    ehdr = load(argv[0], 0);

    entry     = ehdr->e_entry;
    phnum     = ehdr->e_phnum;
    phentsize = ehdr->e_phentsize;
    phaddr    = ehdr->e_phoff + base;
	DEBUG1("entry       ", entry);

    stack = (char *)Mmap(STACKADDR, STACKLEN, PROT_READ | PROT_WRITE,
                 MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0, XNU);
    sp = (void**) &stack[STACKLEN];
	DEBUG1("stack start ", (long)sp);
	DEBUG1("stack end   ", (long)stack);
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
    *--sp = NULL; // End of argv
    sp -= argc; MemMove(sp, argv, argc * 8);
    *(size_t*) --sp = argc;

	DEBUG1("launch      ", entry);
    Launch(0, entry, sp, 0);
#ifdef __linux__
    if(readfd >= 0)
    {
        dup2(readfd, fileno(stdin));
        close(readfd);
    }
    if(writefd >= 0)
    {
        dup2(writefd, fileno(stdout));
        close(writefd);
    }
    #if defined(__x86_64__)
    asm volatile("mov %0, %%rsp;"
                 "jmp *%1;"
                 : : "r"(sp), "r"(ldentry));
    #elif defined(__aarch64__)
    asm volatile("mov sp, %0;"
                 "br  %1;"
                 : : "r"(sp), "r"(ldentry) : "x0");
    #endif
#endif
    __builtin_unreachable();
}

#ifdef __linux__
void runline(char* cmd)
{
    int* argc;
    char*** argv, * saveptr = NULL;
    int count;
    int pipefds[2], writefd, readfd = -1;

    argc = NULL;
    argv = NULL;
    strtok_r(cmd, "|", &saveptr);
    count = 0;
    do
    {
        argv = realloc(argv, (count + 1) * sizeof(*argv));
        argc = realloc(argc, (count + 1) * sizeof(*argc));
        cmd += strspn(cmd, " ");
        argv[count] = NULL;
        argc[count] = 0;

        strtok(cmd, " ");
        do
        {
            argv[count] = realloc(argv[count], ++argc[count] * sizeof(**argv));
            argv[count][argc[count] - 1] = cmd;
        }
        while(cmd = strtok(NULL, " "));

        char* filepath;
        if((filepath = search_path(argv[count][0])) == NULL)
        {
            perror("Error in fstatat");
            free(argv[count]);
            count--;
            continue;
        }
        argv[count][0] = filepath;
        count++;
    }
    while(cmd = strtok_r(NULL, "|", &saveptr));

    for(int i = 0; i < count; ++i)
    {
        if(i > 0)
            readfd = pipefds[0];
        if(i < (count - 1))
        {
            pipe(pipefds);
            writefd = pipefds[1];
        }

        if(fork() == 0)
            run(argc[i], argv[i], readfd, writefd);

        if(i < (count - 1))
            close(writefd);
        writefd = -1;
        if(i > 0)
            close(readfd);

        free(argv[i]);
    }

    for(int i = 0; i < count; ++i)
        wait(NULL);
}

int main()
{
    char interp[128];
    int self;

    char buf[1024];

    ldbase = (Elf64_Addr) ld_addr();
    self = open("/proc/self/exe", O_RDONLY);
    interp[nread(self, interp, sizeof(interp) - 1,
                 search_section(self, ".interp"))] = '\0';
    close(self);
    load(interp, ldbase);

    int terminal = isatty(fileno(stdin));
    while(1)
    {
        if(terminal)
            printf("> ");

        fgets(buf, sizeof buf, stdin);
        if(feof(stdin)) break;
        buf[strcspn(buf, "\n")] = '\0';
        if(strlen(buf) == 0) continue;
        runline(buf);
        wait(NULL);
    }
    puts("\nexit");
    _exit(0);
}
#endif

__attribute__((__noreturn__)) void ApeLoader(long di, long *sp, char dl)
{
    int argc = *sp;
    char **argv = (char **)(sp + 1);
#if 0
    char **envp = (char **)(sp + 1 + argc + 1);
    long *auxv = sp + 1 + argc + 1;
    for (;;) {
        if (!*auxv++)
            break;
    }
#endif
    (void)di;
    (void)dl;

    if (argc > 1)
        run(argc-1, argv+1, 0, 1);
    else Print(XNU, 2, "Usage: load <program> [args...]\n", 0);
    Exit(1, XNU);
}
