/*
 * a simple elf core file backtrace viewer
 *
 * Copyright (C) 2010-2013 Ali Gholami Rudi
 *
 * This program is released under the Modified BSD license.
 */
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "elfcore.h"

/* simplified elf struct and macro names */
#ifdef __x86_64__
#  define Elf_Phdr	Elf64_Phdr
#  define Elf_Ehdr	Elf64_Ehdr
#else
#  define Elf_Phdr	Elf32_Phdr
#  define Elf_Ehdr	Elf32_Ehdr
#endif

#define ALIGN(x,a)		__ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)	(((x)+(mask))&~(mask))

static long readlong(char *mem, unsigned long addr)
{
	Elf_Ehdr *ehdr = (void *) mem;
	Elf_Phdr *phdr = (void *) (mem + ehdr->e_phoff);
	int i;
	for (i = 0; i < ehdr->e_phnum; i++) {
		unsigned long beg = phdr[i].p_vaddr;
		if (addr >= beg && addr < beg + phdr[i].p_filesz) {
			unsigned long diff = addr - beg;
			return *(long *) (mem + phdr[i].p_offset + diff);
		}
	}
	return 0;
}

static void dump_prstatus(char *mem, struct elf_prstatus *prs)
{
	struct user_regs_struct *regs = (void *) &prs->pr_reg;
	unsigned long bp = regs->bp;
	printf("ip=%lx, sp=%lx, bp=%lx\n\n", regs->ip, regs->sp, regs->bp);
	printf("\t%lx: %lx\n", regs->ip, regs->bp);
	while (bp) {
		printf("\t%lx: %lx\n",
			readlong(mem, bp + sizeof(long)), readlong(mem, bp));
		bp = readlong(mem, bp);
	}
	printf("\n");
}

struct elfnote {
	int namesz;
	int descsz;
	int type;
};

static void dumpnote(char *mem, char *note, int len)
{
	struct elfnote *elfnote;
	char *end = note + len;
	while (note < end) {
		elfnote = (void *) note;
		note += sizeof(*elfnote);
		note += ALIGN(elfnote->namesz, 4);
		if (elfnote->type == NT_PRSTATUS)
			dump_prstatus(mem, (void *) note);
		note += ALIGN(elfnote->descsz, 4);
	}
}

static void dumpelf(char *mem)
{
	Elf_Ehdr *ehdr = (void *) mem;
	Elf_Phdr *phdr = (void *) (mem + ehdr->e_phoff);
	if (ehdr->e_type == ET_CORE && phdr[0].p_type == PT_NOTE)
		dumpnote(mem, mem + phdr[0].p_offset, phdr[0].p_filesz);
}

static void die(char *msg)
{
	perror(msg);
	exit(1);
}

static long filesize(int fd)
{
	struct stat stat;
	fstat(fd, &stat);
	return stat.st_size;
}

int main(int argc, char **argv)
{
	int fd;
	long size;
	void *mem;
	if (argc < 2) {
		printf("Usage: %s path\n", argv[0]);
		return 0;
	}
	fd = open(argv[1], O_RDONLY);
	if (fd == -1)
		die("open");
	size = filesize(fd);
	mem = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (!mem)
		die("mmap");
	dumpelf(mem);
	munmap(mem, size);
	close(fd);
	return 0;
}
