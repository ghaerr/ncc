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
#include "syscalls.c"

#define debug       1
#define STACKADDR   ((void *)(0x080000000 - STACKLEN))  /* stack from 2G downwards */
#define STACKLEN    (2048 * 4096)                       /* 8MB stack */

extern void Launch(void *rdi, long entry, void *sp, int rcx) __attribute__((__noreturn__));

static Elf64_Addr search_section(int fd, char* section);

/* declaring this function static causes call ___bzero */
Elf64_Ehdr *load(char* path, Elf64_Addr rebase)
{
    int fd;
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
        rc = Mmap(rebase + aligned, filesz, prot, MAP_PRIVATE | MAP_FIXED, fd, offset);
		DEBUG1("load mmap   ", rc);
        if(memsz > _filesz)
        {
            void* extra = rebase + aligned + _filesz;
            rc = Mmap(extra, memsz - _filesz, prot, MAP_PRIVATE | MAP_FIXED | MAP_ANON, -1, 0);
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
    Close(fd);
	return &ehdr;
}

static Elf64_Addr search_section(int fd, char* section)
{
    Elf64_Ehdr ehdr;
    Elf64_Shdr* shdr;
    uint16_t shnum;
    uint16_t shstrndx;
    char* shstrtab;

    Pread(fd, &ehdr, sizeof(ehdr), 0);

    shnum = ehdr.e_shnum;
    shdr = alloca(sizeof(*shdr) * shnum);
    Pread(fd, shdr, sizeof(*shdr) * shnum, ehdr.e_shoff);

    shstrndx = ehdr.e_shstrndx;
    shstrtab = alloca(shdr[shstrndx].sh_size);
    Pread(fd, shstrtab, shdr[shstrndx].sh_size, shdr[shstrndx].sh_offset);

    for(int i = 0; i < shnum; ++i)
        if(!StrCmp(&shstrtab[shdr[i].sh_name], section))
        {
            return shdr[i].sh_addr;
        }

    return 0;
}

#if 0
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
static void run(int argc, char** argv, int readfd, int writefd)
{
    Elf64_Addr base = 0x400000;
    Elf64_Ehdr *ehdr;
    uint64_t entry, phnum, phentsize, phaddr;
    uint64_t auxv[8 * 2];
    char* stack;
    char** envp;
    int envc;
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
                 MAP_FIXED | MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
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
    envp = (char **)(argv + argc + 1);
    for (envc = 0; *envp++; envc++);
    if (envc) {
        envp = (char **)(argv + argc + 1);
        sp -= envc; MemMove(sp, envp, envc * 8);
    }

    *--sp = NULL; // End of argv
    sp -= argc; MemMove(sp, argv, argc * 8);
    *(size_t*) --sp = argc;

	DEBUG1("launch      ", entry);
    Launch(0, entry, sp, 0);
#if 0
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

#if 0
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
