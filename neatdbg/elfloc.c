/*
 * elfloc, find elf symbols around a virtual address
 *
 * Copyright (C) 2010-2013 Ali Gholami Rudi
 *
 * This program is released under the Modified BSD license.
 */
#include <ctype.h>
#include "../neatlibc/elf.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* simplified elf struct and macro names */
#ifdef __x86_64__
#  define Elf_Phdr	Elf64_Phdr
#  define Elf_Ehdr	Elf64_Ehdr
#  define Elf_Shdr	Elf64_Shdr
#  define Elf_Sym	Elf64_Sym
#else
#  define Elf_Phdr	Elf32_Phdr
#  define Elf_Ehdr	Elf32_Ehdr
#  define Elf_Shdr	Elf32_Shdr
#  define Elf_Sym	Elf32_Sym
#endif

#define MIN(a, b)	((a) < (b) ? (a) : (b))

static long filesize(int fd)
{
	struct stat stat;
	fstat(fd, &stat);
	return stat.st_size;
}

static char *fileread(char *path)
{
	int fd = open(path, O_RDONLY);
	int size = filesize(fd);
	char *buf = malloc(size);
	read(fd, buf, size);
	close(fd);
	return buf;
}

static long htol(char *t)
{
	long n = 0;
	while (*t && isalnum(*t)) {
		n <<= 4;
		if (*t >= '0' && *t <= '9')
			n |= *t - '0';
		if (*t >= 'a' && *t <= 'f')
			n |= *t - 'a' + 10;
		t++;
	}
	return n;
}

static Elf_Ehdr *ehdr;
static Elf_Phdr *phdr;
static Elf_Shdr *shdr;
static Elf_Sym *syms;
static int nsyms;
static char *symstr;

static void init_data(void *mem)
{
	int i;
	ehdr = mem;
	shdr = ehdr->e_shnum ? mem + ehdr->e_shoff : NULL;
	phdr = ehdr->e_phnum ? mem + ehdr->e_phoff : NULL;

	for (i = 0; i < ehdr->e_shnum; i++) {
		if (shdr[i].sh_type == SHT_SYMTAB) {
			syms = mem + shdr[i].sh_offset;
			nsyms = shdr[i].sh_size / sizeof(syms[0]);
			symstr = mem + shdr[shdr[i].sh_link].sh_offset;
		}
	}
}

static int mem_to_off(unsigned long vaddr)
{
	int i;
	for (i = 0; i < ehdr->e_phnum; i++) {
		unsigned long base = phdr[i].p_vaddr;
		if (vaddr >= base && vaddr <= base + phdr[i].p_memsz)
			return phdr[i].p_offset + MIN(phdr[i].p_filesz,
							vaddr - base);
	}
	return 0;
}

static int sym_to_mem(Elf_Sym *sym)
{
	if (sym->st_shndx == SHN_UNDEF || sym->st_shndx == SHN_COMMON)
		return 0;
	return sym->st_value;
}

static Elf_Sym *get_sym(char *name)
{
	int i;
	for (i = 0; i < nsyms; i++)
		if (!strcmp(name, symstr + syms[i].st_name) &&
				syms[i].st_shndx != SHN_UNDEF)
			return &syms[i];
	return NULL;
}

static int sec_region(unsigned long vaddr, unsigned long *beg, unsigned long *end)
{
	int i;
	for (i = 0; i < ehdr->e_phnum; i++) {
		unsigned long base = phdr[i].p_vaddr;
		if (vaddr >= base && vaddr < base + phdr[i].p_memsz) {
			*beg = base;
			*end = base + phdr[i].p_filesz;
			return 0;
		}
	}
	return 1;
}

static void boundaries(unsigned long vaddr)
{
	char *beg_sym = NULL;
	char *end_sym = NULL;
	unsigned long beg = 0l;
	unsigned long end = -1l;
	int i;
	sec_region(vaddr, &beg, &end);
	for (i = 0; i < nsyms; i++) {
		unsigned long symaddr = sym_to_mem(&syms[i]);
		char *name = symstr + syms[i].st_name;
		if (!symaddr || !*name)
			continue;
		if (symaddr <= vaddr && symaddr >= beg) {
			beg_sym = name;
			beg = symaddr;
		}
		if (symaddr > vaddr && symaddr <= end) {
			end_sym = name;
			end = symaddr;
		}
	}
	printf("%s\t+%lx\t%lx\t%d\n",
		beg_sym ? beg_sym : "NULL",
		vaddr - beg, beg, mem_to_off(beg));
	printf("%s\t-%lx\t%lx\t%d\n",
		end_sym ? end_sym : "NULL",
		end - vaddr, end, mem_to_off(end));
}

int main(int argc, char **argv)
{
	unsigned long addr;
	char *name;
	char *buf;
	if (argc < 3) {
		printf("usage: %s addr elf\n", argv[0]);
		return 1;
	}
	buf = fileread(argv[argc - 1]);
	init_data(buf);
	name = argv[1];
	if (name[0] == '=') {
		Elf_Sym *sym = get_sym(name + 1);
		if (!sym)
			return 1;
		addr = sym_to_mem(sym);
	} else {
		addr = htol(argv[1]);
	}
	if (syms)
		boundaries(addr);
	free(buf);
	return 0;
}
