/*-*- mode:unix-assembly; indent-tabs-mode:t; tab-width:8; coding:utf-8     -*-│
│vi: set et ft=asm ts=8 tw=8 fenc=utf-8                                     :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2023 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/

//	Invokes system call.
//
//	long SystemCall(long rdi, long rsi, long rdx, long rcx_r10, long r8, long r9,
//			long arg7_8rsp, int ax_16rsp);
//
//	This function has eight parameters. The first seven are for
//	arguments passed along to the system call. The eight is for
//	the magic number that indicates which system call is called
//
//	The return value follows the Linux kernel convention, where
//	errors are returned as `-errno`. BSD systems are normalized
//	to follow this convention automatically.
	.section	__TEXT,__text,regular,pure_instructions
	.p2align	4, 0x90
	.globl _SystemCall
_SystemCall:
	mov	%rcx,%r10
	mov	16(%rsp),%eax
	clc
	syscall
	jnc	1f
	neg	%rax
1:	ret

	.globl _SystemCallFork
_SystemCallFork:
	mov	%rdi,%rax
	clc
	syscall
	jnc	1f
	mov     $-1,%rax
	ret
1:	or	%edx,%edx	// EDX zero in parent
	jz	9f
	xor	%rax,%rax
9:	ret
