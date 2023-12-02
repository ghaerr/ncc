/*
 * NEATLD ARM/X86(-64) STATIC LINKER
 *
 * Copyright (C) 2010-2023 Ali Gholami Rudi
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <ctype.h>
#include "../neatlibc/elf.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#define DEBUG(...)
//#define DEBUG		debug

#define I_CS		0
#define I_DS		1
#define I_BSS		2

static unsigned long sec_vaddr[3] = {0x400000};	/* virtual address of sections */
static unsigned long sec_laddr[3] = {0x400000};	/* load address of sections */
static int sec_set[3] = {1};			/* set address for section */
static int secalign = 16;			/* section alignment */
static char *entry = "_start";			/* entry symbol */
static int e_machine;				/* target machine */
static int e_flags;				/* elf ehdr flags */
static int undefs;

#define MAXSECS		(1 << 10)
#define MAXOBJS		(1 << 7)
#define MAXSYMS		(1 << 12)
#define PAGE_SIZE	(1 << 12)
#define PAGE_MASK	(PAGE_SIZE - 1)
#define MAXFILES	(1 << 8)
#define MAXPHDRS	4

#define MAX(a, b)	((a) > (b) ? (a) : (b))
#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))

/* simplified elf struct and macro names */
#ifdef __x86_64__
#  define Elf_Phdr	Elf64_Phdr
#  define Elf_Ehdr	Elf64_Ehdr
#  define Elf_Shdr	Elf64_Shdr
#  define Elf_Sym	Elf64_Sym
#  define Elf_Rela	Elf64_Rela
#  define SHT_REL_	SHT_RELA
#  define REL_ADDEND(r)	((r)->r_addend)
#  define ELF_ST_INFO	ELF64_ST_INFO
#  define ELF_ST_BIND	ELF64_ST_BIND
#  define ELF_R_SYM	ELF64_R_SYM
#  define ELF_R_TYPE	ELF64_R_TYPE
#  define ELF_ST_TYPE	ELF64_ST_TYPE
#else
#  define Elf_Phdr	Elf32_Phdr
#  define Elf_Ehdr	Elf32_Ehdr
#  define Elf_Shdr	Elf32_Shdr
#  define Elf_Sym	Elf32_Sym
#  define Elf_Rela	Elf32_Rel
#  define SHT_REL_	SHT_REL
#  define REL_ADDEND(r)	(0)
#  define ELF_ST_INFO	ELF32_ST_INFO
#  define ELF_ST_BIND	ELF32_ST_BIND
#  define ELF_R_SYM	ELF32_R_SYM
#  define ELF_R_TYPE	ELF32_R_TYPE
#  define ELF_ST_TYPE	ELF32_ST_TYPE
#endif

struct obj {
	char *mem;
	Elf_Ehdr *ehdr;
	Elf_Shdr *shdr;
	Elf_Sym *syms;
	int nsyms;
	char *symstr;
	char *shstr;
};

struct secmap {
	Elf_Shdr *o_shdr;
	struct obj *obj;
	unsigned long vaddr;
	unsigned long faddr;
};

struct bss_sym {
	Elf_Sym *sym;
	int off;
};

struct outelf {
	Elf_Ehdr ehdr;
	Elf_Phdr phdr[MAXSECS];
	int nph;
	struct secmap secs[MAXSECS];
	int nsecs;
	struct obj objs[MAXOBJS];
	int nobjs;

	/* code section */
	unsigned long code_addr;

	/* bss section */
	struct bss_sym bss_syms[MAXSYMS];
	int nbss_syms;
	unsigned long bss_vaddr;
	int bss_len;

	/* symtab section */
	Elf_Shdr shdr[MAXSECS];         //FIXME this is too large?
	int nsh;
	char symstr[MAXSYMS];
	Elf_Sym syms[MAXSYMS];
	int nsyms;
	int nsymstr;
	unsigned long shdr_faddr;
	unsigned long syms_faddr;
	unsigned long symstr_faddr;
};

static int nosyms = 0;

void debug(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static Elf_Sym *obj_find(struct obj *obj, char *name)
{
	int i;
	for (i = 0; i < obj->nsyms; i++) {
		Elf_Sym *sym = &obj->syms[i];
		if (ELF_ST_BIND(sym->st_info) == STB_LOCAL ||
				sym->st_shndx == SHN_UNDEF)
			continue;
		if (!strcmp(name, obj->symstr + sym->st_name))
			return sym;
	}
	return NULL;
}

static void obj_init(struct obj *obj, char *mem)
{
	int i;
	obj->mem = mem;
	obj->ehdr = (void *) mem;
	obj->shdr = (void *) (mem + obj->ehdr->e_shoff);
	obj->shstr = mem + obj->shdr[obj->ehdr->e_shstrndx].sh_offset;
	for (i = 0; i < obj->ehdr->e_shnum; i++) {
		if (obj->shdr[i].sh_type != SHT_SYMTAB)
			continue;
		obj->symstr = mem + obj->shdr[obj->shdr[i].sh_link].sh_offset;
		obj->syms = (void *) (mem + obj->shdr[i].sh_offset);
		obj->nsyms = obj->shdr[i].sh_size / sizeof(*obj->syms);
	}
}

static void outelf_init(struct outelf *oe)
{
	memset(oe, 0, sizeof(*oe));
	oe->ehdr.e_ident[0] = 0x7f;
	oe->ehdr.e_ident[1] = 'E';
	oe->ehdr.e_ident[2] = 'L';
	oe->ehdr.e_ident[3] = 'F';
	oe->ehdr.e_ident[4] = sizeof(long) == 8 ? ELFCLASS64 : ELFCLASS32;
	oe->ehdr.e_ident[5] = ELFDATA2LSB;
	oe->ehdr.e_ident[6] = EV_CURRENT;
	oe->ehdr.e_type = ET_EXEC;
	oe->ehdr.e_version = EV_CURRENT;
	oe->ehdr.e_shstrndx = SHN_UNDEF;
	oe->ehdr.e_ehsize = sizeof(oe->ehdr);
	oe->ehdr.e_phentsize = sizeof(oe->phdr[0]);
	oe->ehdr.e_shentsize = sizeof(Elf_Shdr);
}

static struct secmap *outelf_mapping(struct outelf *oe, Elf_Shdr *shdr)
{
	int i;
	for (i = 0; i < oe->nsecs; i++)
		if (oe->secs[i].o_shdr == shdr)
			return &oe->secs[i];
	return NULL;
}

static int outelf_find(struct outelf *oe, char *name,
			struct obj **sym_obj, Elf_Sym **sym_sym)
{
	int i;
	for (i = 0; i < oe->nobjs; i++) {
		struct obj *obj = &oe->objs[i];
		Elf_Sym *sym;
		if ((sym = obj_find(obj, name))) {
			*sym_obj = obj;
			*sym_sym = sym;
			return 0;
		}
	}
	return 1;
}

static unsigned long bss_addr(struct outelf *oe, Elf_Sym *sym)
{
	int i;
	for (i = 0; i < oe->nbss_syms; i++)
		if (oe->bss_syms[i].sym == sym)
			return oe->bss_vaddr + oe->bss_syms[i].off;
	return 0;
}

static void die(char *msg)
{
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

static void die_undef(char *name)
{
	fprintf(stderr, "%s undefined\n", name);
	exit(1);
}

static unsigned long symval(struct outelf *oe, struct obj *obj, Elf_Sym *sym)
{
	struct secmap *sec;
	char *name = obj ? obj->symstr + sym->st_name : NULL;
	int s_idx, s_off;
	switch (ELF_ST_TYPE(sym->st_info)) {
	case STT_SECTION:
		if ((sec = outelf_mapping(oe, &obj->shdr[sym->st_shndx])))
			return sec->vaddr;
		break;
	case STT_NOTYPE:
	case STT_OBJECT:
	case STT_FUNC:
		if (name && *name && sym->st_shndx == SHN_UNDEF)
			if (outelf_find(oe, name, &obj, &sym)) {
				undefs++;
				fprintf(stderr, "%s undefined\n", name);
				return 0;
			}
		if (sym->st_shndx == SHN_COMMON)
			return bss_addr(oe, sym);
		s_idx = sym->st_shndx;
		s_off = sym->st_value;
		if ((sec = outelf_mapping(oe, &obj->shdr[s_idx])))
			return sec->vaddr + s_off;
	}
	return 0;
}

static unsigned long outelf_addr(struct outelf *oe, char *name)
{
	struct obj *obj;
	Elf_Sym *sym;
	if (outelf_find(oe, name, &obj, &sym))
		die_undef(name);
	return symval(oe, obj, sym);
}

#define REL_ARM		0x10000
#define REL_X64		0x20000
#define REL_X86		0x40000

static int arch_rel(int r)
{
	if (e_machine == EM_ARM)
		return REL_ARM | r;
	if (e_machine == EM_X86_64)
		return REL_X64 | r;
	if (e_machine == EM_386)
		return REL_X86 | r;
	return 0;
}

static void outelf_reloc_sec(struct outelf *oe, int o_idx, int s_idx)
{
	struct obj *obj = &oe->objs[o_idx];
	Elf_Shdr *rel_shdr = &obj->shdr[s_idx];
	Elf_Rela *rels = (void *) obj->mem + obj->shdr[s_idx].sh_offset;
	Elf_Shdr *other_shdr = &obj->shdr[rel_shdr->sh_info];
	void *other = (void *) obj->mem + other_shdr->sh_offset;
	int nrels = rel_shdr->sh_size / sizeof(*rels);
	unsigned long addr;
	int i;
	for (i = 0; i < nrels; i++) {
		Elf_Rela *rel = (void *) &rels[i];
		int sym_idx = ELF_R_SYM(rel->r_info);
		Elf_Sym *sym = &obj->syms[sym_idx];
		unsigned long val = symval(oe, obj, sym) + REL_ADDEND(rel);
		unsigned long *dst = other + rel->r_offset;
		switch (arch_rel(ELF_R_TYPE(rel->r_info))) {
		case REL_ARM | R_ARM_NONE:
		case REL_X86 | R_386_NONE:
		case REL_X64 | R_X86_64_NONE:
			break;
		case REL_ARM | R_ARM_ABS16:
		case REL_X86 | R_386_16:
		case REL_X64 | R_X86_64_16:
			*(unsigned short *) dst += val;
			break;
		case REL_ARM | R_ARM_ABS32:
		case REL_X86 | R_386_32:
		case REL_X64 | R_X86_64_32:
		case REL_X64 | R_X86_64_32S:
			*(unsigned int *) dst += val;
			break;
		case REL_X64 | R_X86_64_64:
			*dst += val;
			break;
		case REL_ARM | R_ARM_REL32:
		case REL_ARM | R_ARM_PLT32:
		case REL_X86 | R_386_PLT32:
		case REL_X86 | R_386_PC32:
		case REL_X64 | R_X86_64_PC32:
			addr = outelf_mapping(oe, other_shdr)->vaddr +
				rel->r_offset;
			*(unsigned int *) dst += val - addr;
			break;
		case REL_ARM | R_ARM_PC24:
			addr = outelf_mapping(oe, other_shdr)->vaddr +
				rel->r_offset;
			*dst = (*dst & 0xff000000) |
					((*dst + ((val - addr) >> 2)) & 0x00ffffff);
			break;
		default:
			die("neatld: unknown relocation type!");
		}
	}
}

static void outelf_reloc(struct outelf *oe)
{
	int i, j;
	for (i = 0; i < oe->nobjs; i++) {
		struct obj *obj = &oe->objs[i];
		for (j = 0; j < obj->ehdr->e_shnum; j++)
			if (obj->shdr[j].sh_type == SHT_REL_)
				outelf_reloc_sec(oe, i, j);
	}
}

static void alloc_common(struct outelf *oe, Elf_Sym *sym)
{
	int n = oe->nbss_syms++;
	int off = ALIGN(oe->bss_len, MAX(sym->st_size, 4));
	oe->bss_syms[n].sym = sym;
	oe->bss_syms[n].off = off;
	oe->bss_len = off + sym->st_size;
}

static void outelf_common(struct outelf *oe)
{
	int i, j;
	for (i = 0; i < oe->nobjs; i++) {
		struct obj *obj = &oe->objs[i];
		for (j = 0; j < obj->nsyms; j++) {
			/*Elf_Sym *sym = &obj->syms[j];*/
			if (obj->syms[j].st_shndx == SHN_COMMON) {
				/*printf("COMM %s st_size %ld st_value %ld\n",
					obj->symstr + sym->st_name, (long)sym->st_size,
					(long)sym->st_value);*/
				alloc_common(oe, &obj->syms[j]);
			} else {
				/*int type = ELF_ST_TYPE(sym->st_info);
				Elf_Shdr *shdr = &obj->shdr[sym->st_shndx];
				if (type == STT_OBJECT && shdr->sh_type == SHT_NOBITS) {
					DEBUG("BSS %s st_size %ld st_value %ld st_shndx %d\n",
						obj->symstr + sym->st_name,
						(long)sym->st_size, (long)sym->st_value,
						sym->st_shndx);
				}*/
			}
		}
	}
}

#define SEC_CODE(s)	((s)->sh_flags & SHF_EXECINSTR)
#define SEC_BSS(s)	((s)->sh_type == SHT_NOBITS)
#define SEC_DATA(s)	(!SEC_CODE(s) && !SEC_BSS(s))

static int outelf_str(struct outelf *oe, char *s)
{
	int n = oe->nsymstr;
	char *d = oe->symstr + oe->nsymstr;
	while (*s)
		*d++ = *s++;
	*d++ = '\0';
	oe->nsymstr = d - oe->symstr;
	return n;
}

static void build_symtab(struct outelf *oe)
{
	int i, j;
	Elf_Sym *syms = oe->syms;
	Elf_Shdr *cs_shdr = &oe->shdr[++oe->nsh];
	Elf_Shdr *ds_shdr = &oe->shdr[++oe->nsh];
	Elf_Shdr *bss_shdr = &oe->shdr[++oe->nsh];
	Elf_Shdr *sym_shdr = &oe->shdr[++oe->nsh];
	Elf_Shdr *str_shdr = &oe->shdr[++oe->nsh];
	int n = 1;
	int faddr = oe->shdr_faddr;
	oe->nsh++;
	outelf_str(oe, "");
	sym_shdr->sh_name = outelf_str(oe, ".symtab");
	str_shdr->sh_name = outelf_str(oe, ".strtab");
	cs_shdr->sh_name = outelf_str(oe, ".text");
	ds_shdr->sh_name = outelf_str(oe, ".data");
	bss_shdr->sh_name = outelf_str(oe, ".bss");
	for (i = 0; i < oe->nobjs; i++) {
		struct obj *obj = &oe->objs[i];
		for (j = 0; j < obj->nsyms; j++) {
			Elf_Sym *sym = &obj->syms[j];
			int type = ELF_ST_TYPE(sym->st_info);
			int bind = ELF_ST_BIND(sym->st_info);
			char *name = obj->symstr + sym->st_name;
			if (!*name || bind == STB_LOCAL ||
					sym->st_shndx == SHN_UNDEF)
				continue;
			syms[n].st_name = outelf_str(oe, name);
			syms[n].st_info = ELF_ST_INFO(bind, type);
			syms[n].st_value = symval(oe, obj, sym);
			syms[n].st_size = sym->st_size;
			syms[n].st_shndx = SHN_ABS;
			n++;
		}
	}
	oe->nsyms = n;

	oe->shdr_faddr = faddr;
	faddr += oe->nsh * sizeof(oe->shdr[0]);
	oe->syms_faddr = faddr;
	faddr += oe->nsyms * sizeof(oe->syms[0]);
	oe->symstr_faddr = faddr;
	faddr += oe->nsymstr;

	oe->ehdr.e_shstrndx = str_shdr - oe->shdr;
	oe->ehdr.e_shoff = oe->shdr_faddr;
	oe->ehdr.e_shnum = oe->nsh;

	str_shdr->sh_type = SHT_STRTAB;
	str_shdr->sh_offset = oe->symstr_faddr;
	str_shdr->sh_size = oe->nsymstr;

	sym_shdr->sh_type = SHT_SYMTAB;
	sym_shdr->sh_entsize = sizeof(oe->syms[0]);
	sym_shdr->sh_offset = oe->syms_faddr;
	sym_shdr->sh_size = oe->nsyms * sizeof(oe->syms[0]);
	sym_shdr->sh_link = str_shdr - oe->shdr;
	sym_shdr->sh_info = 1;      /* index of first global */

	cs_shdr->sh_type = SHT_PROGBITS;
	cs_shdr->sh_flags = SHF_ALLOC | SHF_EXECINSTR;
	cs_shdr->sh_offset = oe->phdr[I_CS].p_offset;
	cs_shdr->sh_addr = oe->phdr[I_CS].p_vaddr;
	cs_shdr->sh_addralign = oe->phdr[I_CS].p_align;
	cs_shdr->sh_size = oe->phdr[I_CS].p_filesz;

	ds_shdr->sh_type = SHT_PROGBITS;
	ds_shdr->sh_flags = SHF_ALLOC | SHF_WRITE;
	ds_shdr->sh_offset = oe->phdr[I_DS].p_offset;
	ds_shdr->sh_addr = oe->phdr[I_DS].p_vaddr;
	ds_shdr->sh_addralign = oe->phdr[I_DS].p_align;
	ds_shdr->sh_size = oe->phdr[I_DS].p_filesz;

	bss_shdr->sh_type = SHT_NOBITS;
	bss_shdr->sh_flags = SHF_ALLOC | SHF_WRITE;
	bss_shdr->sh_offset = oe->phdr[I_BSS].p_offset;
	bss_shdr->sh_addr = oe->phdr[I_BSS].p_vaddr;
	bss_shdr->sh_addralign = oe->phdr[I_BSS].p_align;
	bss_shdr->sh_size = oe->phdr[I_BSS].p_filesz;
}

static void outelf_write(struct outelf *oe, int fd)
{
	int i;
	oe->ehdr.e_entry = outelf_addr(oe, entry) -
				sec_vaddr[I_CS] + sec_laddr[I_CS];
	if (!nosyms)
		build_symtab(oe);
	oe->ehdr.e_phnum = oe->nph;
	oe->ehdr.e_phoff = sizeof(oe->ehdr);
	oe->ehdr.e_machine = e_machine;
	oe->ehdr.e_flags = e_flags;
	lseek(fd, 0, SEEK_SET);
	write(fd, &oe->ehdr, sizeof(oe->ehdr));
	write(fd, &oe->phdr, oe->nph * sizeof(oe->phdr[0]));
	for (i = 0; i < oe->nsecs; i++) {
		struct secmap *sec = &oe->secs[i];
		char *buf = sec->obj->mem + sec->o_shdr->sh_offset;
		int len = sec->o_shdr->sh_size;
		if (SEC_BSS(sec->o_shdr))
			continue;
		lseek(fd, sec->faddr, SEEK_SET);
		write(fd, buf, len);
	}
	if (!nosyms) {
		lseek(fd, oe->shdr_faddr, SEEK_SET);
		write(fd, &oe->shdr, oe->nsh * sizeof(oe->shdr[0]));
		lseek(fd, oe->syms_faddr, SEEK_SET);
		write(fd, &oe->syms, oe->nsyms * sizeof(oe->syms[0]));
		lseek(fd, oe->symstr_faddr, SEEK_SET);
		write(fd, &oe->symstr, oe->nsymstr);
	}
}

static int outelf_add(struct outelf *oe, char *mem)
{
	Elf_Ehdr *ehdr = (void *) mem;
	Elf_Shdr *shdr = (void *) (mem + ehdr->e_shoff);
	struct obj *obj;
	int i;
	if (mem[0] != 0x7f && mem[1] != 0x45) {
		DEBUG("Not ELF\n");
		return 0;
	}
	if (ehdr->e_type != ET_REL) {
		DEBUG("Not ET_REL\n");
		return 0;
	}
	e_machine = ehdr->e_machine;
	e_flags = ehdr->e_flags;
	if (oe->nobjs >= MAXOBJS)
		die("neatld: MAXOBJS reached!");
	obj = &oe->objs[oe->nobjs++];
	obj_init(obj, mem);
	for (i = 0; i < ehdr->e_shnum; i++) {
		struct secmap *sec;
		if (!(shdr[i].sh_flags & 0x7))
			continue;
		if (oe->nsecs >= MAXSECS)
			die("neatld: MAXSECS reached!");
		sec = &oe->secs[oe->nsecs++];
		sec->o_shdr = &shdr[i];
		sec->obj = obj;
	}
	return 1;
}

static int link_cs(struct outelf *oe, Elf_Phdr *phdr, unsigned long faddr,
			unsigned long vaddr, unsigned long laddr, int len)
{
	int i;
	for (i = 0; i < oe->nsecs; i++) {
		struct secmap *sec = &oe->secs[i];
		int alignment = MAX(sec->o_shdr->sh_addralign, 4);
		if (!SEC_CODE(sec->o_shdr))
			continue;
		len = ALIGN(vaddr + len, alignment) - vaddr;
		sec->vaddr = vaddr + len;
		sec->faddr = faddr + len;
		len += sec->o_shdr->sh_size;
	}
	phdr->p_type = PT_LOAD;
	phdr->p_flags = PF_R | PF_X;
	phdr->p_vaddr = vaddr;
	phdr->p_paddr = laddr;
	phdr->p_offset = faddr;
	phdr->p_filesz = len;
	phdr->p_memsz = len;
	phdr->p_align = PAGE_SIZE;
	return len;
}

static int link_ds(struct outelf *oe, Elf_Phdr *phdr, unsigned long faddr,
			unsigned long vaddr, unsigned long laddr)
{
	int len = 0;
	int i;
	for (i = 0; i < oe->nsecs; i++) {
		struct secmap *sec = &oe->secs[i];
		if (!SEC_DATA(sec->o_shdr))
			continue;
		sec->vaddr = vaddr + len;
		sec->faddr = faddr + len;
		len += sec->o_shdr->sh_size;
	}
	len = ALIGN(len, 4);
	phdr->p_type = PT_LOAD;
	phdr->p_flags = PF_R | PF_W;
	phdr->p_align = PAGE_SIZE;
	phdr->p_vaddr = vaddr;
	phdr->p_paddr = laddr;
	phdr->p_filesz = len;
	phdr->p_memsz = len;
	phdr->p_offset = faddr;
	return len;
}

static int link_bss(struct outelf *oe, Elf_Phdr *phdr,
			unsigned long faddr, unsigned long vaddr, int len)
{
	int i;
	for (i = 0; i < oe->nsecs; i++) {
		struct secmap *sec = &oe->secs[i];
		int alignment = MAX(sec->o_shdr->sh_addralign, 4);
		if (!SEC_BSS(sec->o_shdr))
			continue;
		len = ALIGN(vaddr + len, alignment) - vaddr;
		sec->vaddr = vaddr + len;
		sec->faddr = faddr;
		len += sec->o_shdr->sh_size;
	}
	phdr->p_type = PT_LOAD;
	phdr->p_flags = PF_R | PF_W;
	phdr->p_vaddr = vaddr;
	phdr->p_paddr = vaddr;
	phdr->p_offset = faddr;
	phdr->p_filesz = 0;
	phdr->p_memsz = len;
	phdr->p_align = PAGE_SIZE;
	return len;
}

static void outelf_link(struct outelf *oe)
{
	unsigned long faddr, vaddr, laddr;
	int len;
	len = ALIGN(sizeof(oe->ehdr) + MAXPHDRS * sizeof(oe->phdr[0]), secalign);
	faddr = len & ~PAGE_MASK;
	vaddr = sec_vaddr[I_CS];
	laddr = sec_laddr[I_CS];
	len = link_cs(oe, &oe->phdr[0], faddr, vaddr, laddr, len & PAGE_MASK);

	len = ALIGN(faddr + len, secalign) - faddr;
	faddr += len;
	vaddr = sec_set[I_DS] ? sec_vaddr[I_DS] | (faddr & PAGE_MASK) :
		vaddr + PAGE_SIZE + len;
	laddr = sec_set[I_DS] ? sec_laddr[I_DS] | (faddr & PAGE_MASK) :
		laddr + PAGE_SIZE + len;
	len = link_ds(oe, &oe->phdr[1], faddr, vaddr, laddr);

	len = ALIGN(faddr + len, secalign) - faddr;
	faddr += len;
	vaddr = sec_set[I_BSS] ? sec_vaddr[I_BSS] | (faddr & PAGE_MASK) :
		vaddr + PAGE_SIZE + len;
	outelf_common(oe);
	oe->bss_vaddr = vaddr;
	len = link_bss(oe, &oe->phdr[2], faddr, vaddr, oe->bss_len);

	oe->nph = 3;
	outelf_reloc(oe);
	oe->shdr_faddr = faddr;
}

struct arhdr {
	char ar_name[16];
	char ar_date[12];
	char ar_uid[6];
	char ar_gid[6];
	char ar_mode[8];
	char ar_size[10];
	char ar_fmag[2];
};

static int get_be32(unsigned char *s)
{
	return s[3] | (s[2] << 8) | (s[1] << 16) | (s[0] << 24);
}

static int sym_defined(struct outelf *oe, char *name, int skipobj)
{
        int i, j;
        for (i = 0; i < oe->nobjs; i++) {
                struct obj *obj = &oe->objs[i];
                if (i == skipobj) continue;
                for (j = 0; j < obj->nsyms; j++) {
                        Elf_Sym *sym = &obj->syms[j];
                        if (ELF_ST_BIND(sym->st_info) == STB_LOCAL)
                                continue;
                        if (strcmp(name, obj->symstr + sym->st_name))
                                continue;
                        if (sym->st_shndx != SHN_UNDEF)
                                return 1;
                }
        }
        return 0;
}

static int outelf_multidef(struct outelf *oe)
{
        int i, j;
        int multidef = 0;
        for (i = 0; i < oe->nobjs; i++) {
                struct obj *obj = &oe->objs[i];
                for (j = 0; j < obj->nsyms; j++) {
                        Elf_Sym *sym = &obj->syms[j];
                        if (ELF_ST_BIND(sym->st_info) == STB_LOCAL)
                                continue;
                        if (sym->st_shndx != SHN_UNDEF) {
                                if (sym_defined(oe, obj->symstr + sym->st_name, i)) {
                                        printf("%s multiply defined\n",
                                                obj->symstr + sym->st_name);
                                        multidef++;
                                }
                        }
                }
        }
        return multidef;
}

/* search whether passed symbol is still undefined, return 1 to load archive */
static int sym_undef(struct outelf *oe, char *name, int nobjs)
{
	int i, j;
	int undef = 0;                          /* default undefined but don't load archive */
	for (i = 0; i < nobjs; i++) {
		struct obj *obj = &oe->objs[i];
		for (j = 0; j < obj->nsyms; j++) {
			Elf_Sym *sym = &obj->syms[j];
			if (ELF_ST_BIND(sym->st_info) == STB_LOCAL)
				continue;
			if (strcmp(name, obj->symstr + sym->st_name))
				continue;
			if (sym->st_shndx != SHN_UNDEF)
				return 0;       /* symbol defined, don't load archive */
			undef = 1;              /* symbol defined, load archive */
		}
	}
	return undef;
}

static int outelf_ar_link(struct outelf *oe, char *ar, int base)
{
	char *ar_index;
	char *ar_name;
	int nsyms = get_be32((void *) ar);
	int added = 0;
	int i;
	ar_index = ar + 4;
	ar_name = ar_index + nsyms * 4;
	for (i = 0; i < nsyms; i++) {
		int off = get_be32((void *) ar_index + i * 4) +
				sizeof(struct arhdr);
		if (sym_undef(oe, ar_name, oe->nobjs)) {
			outelf_add(oe, ar - base + off);
			added++;
		}
		ar_name = strchr(ar_name, '\0') + 1;
	}
	return added;
}

static int outelf_ar_macos(struct outelf *oe, char *ar)
{
        int i;
        char *name;
        int added = 0;
        int onsecs = oe->nsecs;
        int onobjs = oe->nobjs;
        if (!outelf_add(oe, ar))
                return 0;
        struct obj *obj = &oe->objs[oe->nobjs-1];
        for (i = 0; i < obj->nsyms; i++) {
                Elf_Sym *sym = &obj->syms[i];
                if (ELF_ST_BIND(sym->st_info) == STB_LOCAL)
                        continue;
                if (sym->st_shndx != SHN_UNDEF) {
                        name = obj->symstr + sym->st_name;
                        DEBUG("%s, ", name);
                        if (sym_undef(oe, name, oe->nobjs-1)) {
                                DEBUG("(Found)\n");
                                added++;
                                break;
                        }
                }
        }
        if (!added) {
                oe->nsecs = onsecs;
                oe->nobjs = onobjs;
        }
        return added;
}

static void outelf_archive(struct outelf *oe, char *ar)
{
	char *beg = ar;
	int count = 1;
	int found = 0;

again:
	/* skip magic */
	ar += 8;
	for(;;) {
		struct arhdr *hdr = (void *) ar;
		char *name = hdr->ar_name;
		int size;
		ar += sizeof(*hdr);
		hdr->ar_size[sizeof(hdr->ar_size) - 1] = '\0';
		size = atoi(hdr->ar_size);
		size = (size + 1) & ~1;
		if (!size && !*name)
			break;
		if (!strncmp(name, "#1/", 3)) {
			name += sizeof(*hdr);
			DEBUG("LIB %s: ", name);
			if (outelf_ar_macos(oe, name + atoi(&hdr->ar_name[3])))
				found++;
		} else {
			if (name[0] == '/' && name[1] == ' ') {
				while (outelf_ar_link(oe, ar, ar - beg))
					;
				break;
			}
			if (name[0] == '/' && name[1] == '/' && name[2] == ' ')
				outelf_add(oe, ar);
		}
		ar += size;
	}
	if (found) {
		printf("(re-read lib %d)\n", ++count);
		found = 0;
		ar = beg;
		goto again;
	}
}

static long filesize(int fd)
{
	struct stat stat;
	fstat(fd, &stat);
	return stat.st_size;
}

static char *fileread(char *path)
{
	int fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "neatld: cannot open %s\n", path);
		return NULL;
	}
	int size = filesize(fd);
	char *buf = malloc(size);
	read(fd, buf, size);
	close(fd);
	return buf;
}

static int is_ar(char *path)
{
	int len = strlen(path);
	return len > 2 && path[len - 2] == '.' && path[len - 1] == 'a';
}

#define LIBDIRS		(1 << 5)
#define PATHLEN		(1 << 8)

static char *libdirs[LIBDIRS] = {"/lib"};
static int nlibdirs = 1;

static int lib_find(char *path, char *lib)
{
	struct stat st;
	int i;
	for (i = 0; i < nlibdirs; i++) {
		sprintf(path, "%s/lib%s.a", libdirs[i], lib);
		if (!stat(path, &st))
			return 0;
	}
	return 1;
}

static unsigned long hexnum(char *s)
{
	unsigned long n = 0;
	if (s[0] == '0' && s[1] == 'x')
		s += 2;
	for (; isdigit(*s) || isalpha(*s); s++) {
		n <<= 4;
		n |= isdigit(*s) ? *s - '0' : tolower(*s) - 'a' + 10;
	}
	return n;
}

static void set_addr(int sec, char *arg)
{
	int idx = I_CS;
	char *sep = strchr(arg, ':');
	if (sec == 'd')
		idx = I_DS;
	if (sec == 'b')
		idx = I_BSS;
	sec_vaddr[idx] = hexnum(arg);
	sec_laddr[idx] = sep ? hexnum(sep + 1) : sec_vaddr[idx];
	sec_set[idx] = 1;
}

static char *obj_add(struct outelf *oe, char *path)
{
	char *buf = fileread(path);
	if (!buf)
		die("neatld: cannot open object!");
	if (is_ar(path))
		outelf_archive(oe, buf);
	else
		outelf_add(oe, buf);
	return buf;
}

int main(int argc, char **argv)
{
	char out[PATHLEN] = "a.out";
	struct outelf oe;
	char *mem[MAXFILES];
	int nmem = 0;
	int fd, multidefs;
	int i = 0;
	if (argc < 2)
		die("neatld: no object given!");
	outelf_init(&oe);

	while (++i < argc) {
		if (argv[i][0] != '-') {
			mem[nmem++] = obj_add(&oe, argv[i]);
			continue;
		}
		if (argv[i][1] == 'l') {
			char path[PATHLEN];
			if (lib_find(path, argv[i] + 2))
				die("neatld: cannot find library!");
			mem[nmem++] = obj_add(&oe, path);
			continue;
		}
		if (argv[i][1] == 'L') {
			libdirs[nlibdirs++] = argv[i][2] ? argv[i] + 2 : argv[++i];
			continue;
		}
		if (argv[i][1] == 'o') {
			strcpy(out, argv[i][2] ? argv[i] + 2 : argv[++i]);
			continue;
		}
		if (argv[i][1] == 's') {
			nosyms = 1;
			continue;
		}
		if (argv[i][1] == 'g')
			continue;
		if (argv[i][1] == 'm') {
			char sec = argv[i][2];
			char *arg = argv[i][3] == '=' ? argv[i] + 4 : argv[++i];
			set_addr(sec, arg);
			continue;
		}
		if (argv[i][1] == 'p') {
			secalign = PAGE_SIZE;
			continue;
		}
		if (argv[i][1] == 'e') {
			entry = argv[i][2] ? argv[i] + 2 : argv[++i];
			continue;
		}
		if (argv[i][1] == 'h') {
			printf("Usage: neatld [options] objects\n\n");
			printf("Options:\n");
			printf("  -o out          set the output file\n");
			printf("  -l lib          link with library lib\n");
			printf("  -L dir          search dir for libraries\n");
			printf("  -s              do not include a symbol table\n");
			printf("  -mXvaddr:laddr  section virtual/load address\n");
			printf("  -p              page-align sections\n");
			printf("  -e              entry point symbol\n");
			return 1;
		}
	}
	outelf_link(&oe);

	if (undefs)
		fprintf(stderr, "%d undefined symbols\n", undefs);
	multidefs = outelf_multidef(&oe);

	if (!multidefs && !undefs) {
		fd = open(out, O_WRONLY | O_TRUNC | O_CREAT, 0700);
		if (fd < 0)
			die("neatld: failed to create the output!");
		outelf_write(&oe, fd);
		close(fd);
	}
	for (i = 0; i < nmem; i++)
		free(mem[i]);
	return multidefs || undefs;
}
