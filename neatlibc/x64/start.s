;default rel            ; default generate rip-relative code

        extern main
        ;extern __neatlibc_exit
        extern _exit
        global _start:function
_start:
	xor	rbp, rbp
	pop	rdi			; argc
	mov	rsi, rsp		; argv
	push	rdi
	lea	rdx, [rsi + rdi * 8 + 8]; envp
	mov	[environ], rdx
	and	rsp, -16		; align rsp

	call	main
	mov	rbx, rax
	;call	__neatlibc_exit
	mov	rdi, rbx
	call    _exit

        SECTION .data
        global  environ:object
environ:dq      0
