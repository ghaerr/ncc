/*
 * neatas - a small arm assembler
 *
 * Copyright (C) 2011 Ali Gholami Rudi
 *
 * This program is released under GNU GPL version 2.
 */
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "out.h"

#define BUFSIZE		(1 << 14)
#define TOKLEN		128

#define DELIMS		",:{}[]#=-+ \t\n/!^"
#define TOK2(a)		((a)[0] << 16 | (a)[1] << 8)
#define TOK3(a)		((a)[0] << 16 | (a)[1] << 8 | (a)[2])

static char src[256];
static char buf[BUFSIZE];
static int cur;

static char cs[BUFSIZE];
static int cslen;

static void gen(unsigned long i)
{
	memcpy(cs + cslen, &i, 4);
	cslen += 4;
}

static int tok_read(char *s)
{
	while (1) {
		while (isspace(buf[cur]))
			cur++;
		if (buf[cur] == '/' && buf[cur + 1] == '*') {
			while (buf[cur] && (buf[cur] != '*' || buf[cur + 1] != '/'))
				cur++;
			continue;
		}
		if (buf[cur] == ';' || buf[cur] == '@') {
			while (buf[cur] && buf[cur] != '\n')
				cur++;
			continue;
		}
		break;
	}
	if (!strchr(DELIMS, buf[cur])) {
		while (!strchr(DELIMS, buf[cur]))
			*s++ = buf[cur++];
		*s = '\0';
		return 0;
	}
	s[0] = buf[cur++];
	s[1] = '\0';
	return s[0] != 0;
}

static char tok[TOKLEN];
static char tokc[TOKLEN];
static int tok_next;

/* next token in lower-case */
static char *tok_get(void)
{
	char *s = tokc;
	char *d = tok;
	if (!tok_next) {
		tok_read(tokc);
		while (*s)
			*d++ = tolower(*s++);
		*d = '\0';
	}
	tok_next = 0;
	return tok;
}

/* next token in original case */
static char *tok_case(void)
{
	tok_get();
	return tokc;
}

/* have a look at the next token */
static char *tok_see(void)
{
	if (!tok_next)
		tok_get();
	tok_next = 1;
	return tok;
}

static char *digs = "0123456789abcdef";

static long num(char *s, int bits)
{
	int b = 10;
	int neg = 0;
	long n = 0;
	if (*s == '-' || *s == '+') {
		neg = *s == '-';
		s++;
	}
	if (s[0] == '0' && s[1] == 'x') {
		b = 16;
		s += 2;
	}
	while (*s) {
		int d = strchr(digs, *s) - digs;
		n *= b;
		n += d;
		s++;
	}
	if (neg)
		n = -n;
	return bits < 32 ? n & ((1ul << bits) - 1) : n;
}

#define NLOCALS		1024
#define NEXTERNS	1024
#define NAMELEN		32

static char locals[NLOCALS][NAMELEN];
static char loffs[NLOCALS];
static int nlocals;
static char externs[NEXTERNS][NAMELEN];
static int nexterns;
static char globals[NEXTERNS][NAMELEN];
static int nglobals;

static void label_extern(char *name)
{
	int idx = nexterns++;
	strcpy(externs[idx], name);
}

static void label_global(char *name)
{
	int idx = nglobals++;
	strcpy(globals[idx], name);
}

static void label_local(char *name)
{
	int idx = nlocals++;
	strcpy(locals[idx], name);
	loffs[idx] = cslen;
	out_sym(locals[idx], OUT_CS, loffs[idx], 0);
}

static int label_isextern(char *name)
{
	int i;
	for (i = 0; i < nexterns; i++)
		if (!strcmp(name, externs[i]))
			return 1;
	return 0;
}

static int label_offset(char *name)
{
	int i;
	for (i = 0; i < nlocals; i++)
		if (!strcmp(name, locals[i]))
			return loffs[i];
	return 0;
}

static void label_write(void)
{
	int i;
	for (i = 0; i < nglobals; i++)
		out_sym(globals[i], OUT_GLOB | OUT_CS,
			label_offset(globals[i]), 0);
}

#define NRELOCS		1024

/* absolute relocations */
static char absns[NRELOCS][NAMELEN];	/* symbol name */
static long absos[NRELOCS];		/* relocation location */
static int nabs;
/* relative relocations */
static char relns[NRELOCS][NAMELEN];	/* symbol name */
static long relos[NRELOCS];		/* relocation location */
static long relas[NRELOCS];		/* relocation addend */
static long relbs[NRELOCS];		/* relocation bits: ldrh=8, 12=ldr, 24=bl */
static int nrel;

static void reloc_rel(char *name, long off, int bits)
{
	int idx = nrel++;
	strcpy(relns[idx], name);
	relos[idx] = cslen;
	relas[idx] = off;
	relbs[idx] = bits;
}

static void reloc_abs(char *name)
{
	int idx = nabs++;
	strcpy(absns[idx], name);
	absos[idx] = cslen;
}

#define CSBEG_NAME		"__neatas_cs"

/* fill immediate value for bl instruction */
static void bl_imm(long *dst, long imm)
{
	imm = ((*dst << 2) + imm) >> 2;
	*dst = (*dst & 0xff000000) | (imm & 0x00ffffff);
}

/* fill immediate value for ldr instruction */
static void ldr_imm(long *dst, long imm, int half)
{
	/* set u-bit for negative offsets */
	if (imm < 0) {
		*dst ^= (1 << 23);
		imm = -imm;
	}
	if (!half)
		*dst = (*dst & 0xfffff000) | ((*dst + imm) & 0x00000fff);
	if (half)
		*dst = (*dst & 0xfffff0f0) |
				(imm & 0x0f) | ((imm & 0xf0) << 4);
}

static void reloc_write(void)
{
	int i;
	out_sym(CSBEG_NAME, OUT_CS, 0, 0);
	for (i = 0; i < nabs; i++) {
		if (label_isextern(absns[i])) {
			out_rel(absns[i], OUT_CS, absos[i]);
		} else {
			long off = label_offset(absns[i]);
			out_rel(CSBEG_NAME, OUT_CS, absos[i]);
			*(long *) (cs + absos[i]) += off;
		}
	}
	for (i = 0; i < nrel; i++) {
		long *dst = (void *) cs + relos[i];
		long off;
		if (label_isextern(relns[i])) {
			out_rel(relns[i], OUT_CS | OUT_REL24, relos[i]);
			bl_imm(dst, relas[i] - 8);
			continue;
		}
		off = relas[i] + label_offset(relns[i]) - relos[i] - 8;
		/* bl instruction */
		if (relbs[i] == 24)
			bl_imm(dst, off);
		else
			ldr_imm(dst, off, relbs[i] == 8);
	}
}

#define NDATS		1024

/* data pool */
static long dat_offs[NDATS];		/* data immediate value */
static long dat_locs[NDATS];		/* address of pointing ldr */
static char dat_names[NDATS][NAMELEN];	/* relocation data symbol name */
static int ndats;

static void pool_num(long num)
{
	int idx = ndats++;
	dat_offs[idx] = num;
	dat_locs[idx] = cslen;
}

static void pool_reloc(char *name, long off)
{
	int idx = ndats++;
	dat_offs[idx] = off;
	dat_locs[idx] = cslen;
	strcpy(dat_names[idx], name);
}

static void pool_write(void)
{
	int i;
	for (i = 0; i < ndats; i++) {
		if (dat_names[i]) {
			long *loc = (void *) cs + dat_locs[i];
			int off = cslen - dat_locs[i] - 8;
			reloc_abs(dat_names[i]);
			/* ldrh needs special care */
			if (*loc & (1 << 26))
				*loc = (*loc & 0xfffff000) | (off & 0x00000fff);
			else
				*loc = (*loc & 0xfffff0f0) | (off & 0x0f) |
					((off & 0xf0) << 4);
		}
		gen(dat_offs[i]);
	}
}

static char *dpops[] = {
	"and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc",
	"tst", "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn"
};

static char *conds[] = {
	"eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc",
	"hi", "ls", "ge", "lt", "gt", "le", "al", "nv"
};

static char *regs[] = {
	"a1", "a2", "a3", "a4", "v1", "v2", "v3", "v4",
	"v5", "v6", "v7", "v8", "ip", "sp", "lr", "pc"
};

static int get_reg(char *s)
{
	int i;
	if (s[0] == 'f' && s[1] == 'p')
		return 11;
	for (i = 0; i < 16; i++)
		if (TOK2(s) == TOK2(regs[i]))
			return i;
	if (s[0] == 'r')
		return atoi(s + 1);
	return -1;
}

static void fill_buf(int fd)
{
	int len = 0;
	int nr;
	while ((nr = read(fd, buf + len, sizeof(buf) - len - 1)) > 0)
		len += nr;
	buf[len] = '\0';
}

static int tok_jmp(char *s)
{
	if (!strcmp(s, tok_see())) {
		tok_get();
		return 0;
	}
	return 1;
}

static void die(char *msg)
{
	int lineno = 1;
	int i;
	for (i = 0; i < cur; i++)
		if (buf[i] == '\n')
			lineno++;
	fprintf(stderr, "%s:%d: %s\n", src, lineno, msg);
	exit(1);
}

static void tok_expect(char *s)
{
	if (strcmp(s, tok_get()))
		die("syntax error");
}

static int get_cond(char *s)
{
	int i;
	if (s[0] == 'h' && s[1] == 's')
		return 2;
	if (s[0] == 'l' && s[1] == 'o')
		return 3;
	for (i = 0; i < 16; i++)
		if (TOK2(s) == TOK2(conds[i]))
			return i;
	return -1;
}

static int add_op(char *s)
{
	int i;
	for (i = 0; i < 16; i++)
		if (TOK3(s) == TOK3(dpops[i]))
			return i;
	return -1;
}

static int shiftmode(char *s)
{
	if (TOK3(s) == TOK3("lsl"))
		return 0;
	if (TOK3(s) == TOK3("lsr"))
		return 1;
	if (TOK3(s) == TOK3("asr"))
		return 2;
	if (TOK3(s) == TOK3("ror"))
		return 3;
	return 0;
}

static int ldr_word(void)
{
	int sm = 0;
	int rm;
	int shifts = 0;
	int u = 1;
	if (tok_jmp(","))
		return 0;
	if (!tok_jmp("#")) {
		u = tok_jmp("-");
		return (u << 23) | num(tok_get(), 12);
	}
	if (!tok_jmp("-"))
		u = 0;
	rm = get_reg(tok_get());
	if (!tok_jmp(",")) {
		sm = shiftmode(tok_get());
		tok_expect("#");
		shifts = num(tok_get(), 8);
	}
	return (1 << 25) | (u << 23) | (shifts << 7) | (sm << 5) | rm;
}

static int ldr_half(int s, int h)
{
	int u, n;
	int o = 0x90 | (s << 6) | (h << 5);
	if (tok_jmp(","))
		return o | (1 << 22);
	if (!tok_jmp("#")) {
		u = tok_jmp("-");
		n = num(tok_get(), 8);
		return o | (1 << 22) | (u << 23) | (n & 0x0f) | ((n & 0xf0) << 4);
	}
	u = tok_jmp("-");
	return o | (u << 23) | get_reg(tok_get());
}

static long ldr_off(void)
{
	long off = 0;
	while (1) {
		if (!tok_jmp("-")) {
			off -= num(tok_get(), 32);
			continue;
		}
		if (!tok_jmp("+")) {
			off += num(tok_get(), 32);
			continue;
		}
		break;
	}
	return off;
}

/*
 * single data transfer:
 * +------------------------------------------+
 * |COND|01|I|P|U|B|W|L| Rn | Rd |   offset   |
 * +------------------------------------------+
 *
 * I: immediate/offset
 * P: post/pre indexing
 * U: down/up
 * B: byte/word
 * W: writeback
 * L: store/load
 * Rn: base register
 * Rd: source/destination register
 *
 * I=1 offset=| immediate |
 * I=0 offset=| shift  | Rm |
 *
 * halfword and signed data transfer
 * +----------------------------------------------+
 * |COND|000|P|U|0|W|L| Rn | Rd |0000|1|S|H|1| Rm |
 * +----------------------------------------------+
 *
 * +----------------------------------------------+
 * |COND|000|P|U|1|W|L| Rn | Rd |off1|1|S|H|1|off2|
 * +----------------------------------------------+
 *
 * S: singed
 * H: halfword
 */
static int ldr(char *cmd)
{
	int l = 0;
	int rd, rn;
	int cond;
	int w = 0;
	int sign = 0;
	int byte = 0;
	int half = 0;
	int o;
	if (TOK3(cmd) != TOK3("ldr") && TOK3(cmd) != TOK3("str"))
		return 1;
	if (TOK3(cmd) == TOK3("ldr"))
		l = 1;
	cond = get_cond(cmd + 3);
	cmd += cond < 0 ? 2 : 5;
	if (cond < 0)
		cond = 14;
	while (*++cmd) {
		if (*cmd == 't')
			w = 1;
		if (*cmd == 'b')
			byte = 1;
		if (*cmd == 'h')
			half = 1;
		if (*cmd == 's')
			sign = 1;
	}
	rd = get_reg(tok_get());
	tok_expect(",");
	o = (cond << 28) | (l << 20) | (rd << 12) | (half << 5) | (sign << 6);
	if (half || sign)
		o |= 0x90;
	else
		o |= (1 << 26) | (byte << 22);
	if (tok_jmp("[")) {
		char sym[NAMELEN];
		rn = 15;
		if (!tok_jmp("=")) {
			strcpy(sym, tok_case());
			pool_reloc(sym, ldr_off());
		} else {
			strcpy(sym, tok_case());
			reloc_rel(sym, ldr_off(), (half || sign) ? 8 : 12);
		}
		if (half || sign)
			o |= (1 << 22);
		else
			o |= (1 << 26);
		gen(o | (1 << 23) | (1 << 24) | (rn << 16));
		return 0;
	}
	rn = get_reg(tok_get());
	o |= (rn << 16);
	if (!tok_jmp("]")) {
		gen(o | (w << 21) | ((half || sign) ? ldr_half(sign, half) :
						ldr_word()));
		return 0;
	}
	o |= (1 << 24) | ((half || sign) ? ldr_half(sign, half) : ldr_word());
	tok_expect("]");
	if (!tok_jmp("!"))
		o |= (1 << 21);
	gen(o);
	return 0;
}

static int ldm_regs(void)
{
	int o = 0;
	tok_expect("{");
	while (1) {
		int r1 = get_reg(tok_get());
		int r2 = r1;
		int i;
		if (!tok_jmp("-"))
			r2 = get_reg(tok_get());
		for (i = r1; i <= r2; i++)
			o |= (1 << i);
		if (tok_jmp(","))
			break;
	}
	tok_expect("}");
	return o;
}

static int ldm_type(char *s, int l)
{
	int p = 0;
	int u = 0;
	if (*s == 'i' || *s == 'd') {
		p = s[0] == 'i';
		u = s[1] == 'b';
	} else {
		p = s[0] == (l ? 'e' : 'f');
		u = s[1] == (l ? 'd' : 'a');
	}
	return (p << 24) | (u << 23);
}

/*
 * block data transfer
 * +----------------------------------------+
 * |COND|100|P|U|S|W|L| Rn |    reg list    |
 * +----------------------------------------+
 *
 * P: post/pre indexing
 * U: down/up
 * S: PSR/user bit
 * W: write back
 * L: load/store
 * Rn: base register
 */
static int ldm(char *cmd)
{
	int rn;
	int cond;
	int l = 0, w = 0, s = 0;
	int o = 4 << 25;
	if (TOK3(cmd) != TOK3("ldm") && TOK3(cmd) != TOK3("stm"))
		return 1;
	if (TOK3(cmd) == TOK3("ldm"))
		l = 1;
	cond = get_cond(cmd + 3);
	o |= ldm_type(cond < 0 ? cmd + 3 : cmd + 5, l);
	rn = get_reg(tok_get());
	if (!tok_jmp("!"))
		w = 1;
	tok_expect(",");
	if (cond < 0)
		cond = 14;
	o |= ldm_regs();
	if (!tok_jmp("^"))
		s = 1;
	gen(o | (cond << 28) | (s << 22) | (w << 21) | (l << 20) | (rn << 16));
	return 0;
}

static int add_encimm(unsigned n)
{
	int i = 0;
	while (i < 12 && (n >> ((4 + i) << 1)))
		i++;
	return (n >> (i << 1)) | (((16 - i) & 0x0f) << 8);
}

static long add_decimm(unsigned n)
{
	int rot = (16 - ((n >> 8) & 0x0f)) & 0x0f;
	return (n & 0xff) << (rot << 1);
}

static int add_op2(void)
{
	int sm, rm;
	if (!tok_jmp("#")) {
		long n = num(tok_get(), 32);
		long imm = add_encimm(n);
		if (add_decimm(imm) != n)
			die("cannot encode immediate");
		return (1 << 25) | imm;
	}
	rm = get_reg(tok_get());
	if (tok_jmp(","))
		return rm;
	sm = shiftmode(tok_get());
	if (!tok_jmp("#"))
		return (num(tok_get(), 4) << 7) | (sm << 5) | (rm << 0);
	return (get_reg(tok_get()) << 8) | (sm << 5) | (1 << 4) | (rm << 0);
}

/*
 * data processing:
 * +---------------------------------------+
 * |COND|00|I| op |S| Rn | Rd |  operand2  |
 * +---------------------------------------+
 *
 * S: set condition code
 * Rn: first operand
 * Rd: destination operand
 *
 * I=0 operand2=| shift  | Rm |
 * I=1 operand2=|rota|  imm   |
 */
static int add(char *cmd)
{
	int op, cond;
	int rd = 0, rn = 0;
	int nops = 2;
	int s = 0;
	op = add_op(cmd);
	if (op < 0)
		return 1;
	cond = get_cond(cmd + 3);
	s = cmd[cond < 0 ? 3 : 6] == 's';
	if (op == 13 || op == 15)
		nops = 1;
	if ((op & 0x0c) == 0x08)
		s = 1;
	if (cond < 0)
		cond = 14;
	if ((op & 0xc) != 0x8) {
		rd = get_reg(tok_get());
		tok_expect(",");
	}
	if (nops > 1) {
		rn = get_reg(tok_get());
		tok_expect(",");
	}
	gen((cond << 28) | (s << 20) | (op << 21) | (rn << 16) | (rd << 12) | add_op2());
	return 0;
}

/*
 * multiply
 * +----------------------------------------+
 * |COND|000000|A|S| Rd | Rn | Rs |1001| Rm |
 * +----------------------------------------+
 *
 * Rd: destination
 * A: accumulate
 * C: set condition codes
 *
 * I=0 operand2=| shift  | Rm |
 * I=1 operand2=|rota|  imm   |
 */
static int mul(char *cmd)
{
	int cond;
	int rd, rm, rs, rn = 0;
	int s = 0;
	int a = 0;
	if (TOK3(cmd) != TOK3("mul") && TOK3(cmd) != TOK3("mla"))
		return 1;
	if (TOK3(cmd) == TOK3("mla"))
		a = 1;
	cond = get_cond(cmd + 3);
	s = cmd[cond < 0 ? 3 : 6] == 's';
	if (cond < 0)
		cond = 14;
	rd = get_reg(tok_get());
	tok_expect(",");
	rm = get_reg(tok_get());
	tok_expect(",");
	rs = get_reg(tok_get());
	if (a) {
		tok_expect(",");
		rn = get_reg(tok_get());
	}
	gen((cond << 28) | (a << 21) | (s << 20) | (rd << 16) |
		(rn << 12) | (rs << 8) | (9 << 4) | (rm << 0));
	return 0;
}

/*
 * software interrupt:
 * +----------------------------------+
 * |COND|1111|                        |
 * +----------------------------------+
 *
 */
static int swi(char *cmd)
{
	int n;
	int cond;
	if (TOK3(cmd) != TOK3("swi"))
		return 1;
	cond = get_cond(cmd + 3);
	if (cond == -1)
		cond = 14;
	tok_jmp("#");
	n = num(tok_get(), 24);
	gen((cond << 28) | (0xf << 24) | n);
	return 0;
}

/*
 * branch:
 * +-----------------------------------+
 * |COND|101|L|         offset         |
 * +-----------------------------------+
 *
 * L: link
 */
static int bl(char *cmd)
{
	int l = 0;
	int cond;
	char sym[NAMELEN];
	if (*cmd++ != 'b')
		return 1;
	if (*cmd == 'l') {
		l = 1;
		cmd++;
	}
	cond = get_cond(cmd);
	if (cond == -1)
		cond = 14;
	strcpy(sym, tok_case());
	reloc_rel(sym, ldr_off(), 24);
	gen((cond << 28) | (5 << 25) | (l << 24));
	return 0;
}

/*
 * move PSR to a register
 * +-------------------------------------+
 * |COND|00010|P|001111| Rd |000000000000|
 * +-------------------------------------+
 *
 * move a register to PSR
 * +--------------------------------------+
 * |COND|00|I|10|P|1010001111| source op  |
 * +--------------------------------------+
 *
 * P: CPSR/SPSR_cur
 *
 * I=0 source=|00000000| Rm |
 * I=1 source=|rot | imm_u8 |
 */
static int msr(char *cmd)
{
	return 1;
}

static int directive(char *cmd)
{
	if (cmd[0] != '.')
		return 1;
	if (!strcmp(".extern", cmd)) {
		label_extern(tok_case());
	}
	if (!strcmp(".global", cmd)) {
		label_global(tok_case());
	}
	if (!strcmp(".word", cmd)) {
		do {
			if (!tok_jmp("=")) {
				reloc_abs(tok_case());
				gen(ldr_off());
			} else {
				gen(num(tok_get(), 32));
			}
		} while (!tok_jmp(","));
	}
	return 0;
}

static int stmt(void)
{
	char first[TOKLEN];
	char first_case[TOKLEN];
	strcpy(first, tok_see());
	strcpy(first_case, tok_case());
	/* a label */
	if (!tok_jmp(":")) {
		label_local(first_case);
		return 0;
	}
	if (!directive(first))
		return 0;
	if (!add(first))
		return 0;
	if (!mul(first))
		return 0;
	if (!ldr(first))
		return 0;
	if (!ldm(first))
		return 0;
	if (!msr(first))
		return 0;
	if (!swi(first))
		return 0;
	if (!bl(first))
		return 0;
	return 1;
}

int main(int argc, char *argv[])
{
	char obj[128] = "";
	int ofd, ifd;
	int i = 1;
	while (i < argc && argv[i][0] == '-') {
		if (argv[i][1] == 'o')
			strcpy(obj, argv[++i]);
		i++;
	}
	if (i == argc) {
		fprintf(stderr, "neatcc: no file given\n");
		return 1;
	}
	strcpy(src, argv[i]);
	ifd = open(src, O_RDONLY);
	fill_buf(ifd);
	close(ifd);
	out_init(0);
	while (!stmt())
		;
	label_write();
	pool_write();
	reloc_write();
	if (!*obj) {
		char *s = obj;
		strcpy(obj, src);
		while (*s && *s != '.')
			s++;
		*s++ = '.';
		*s++ = 'o';
		*s++ = '\0';
	}
	ofd = open(obj, O_WRONLY | O_TRUNC | O_CREAT, 0600);
	out_write(ofd, cs, cslen, cs, 0);
	close(ofd);
	return 0;
}
