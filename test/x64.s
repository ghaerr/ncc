;default rel            ; default generate rip-relative code

        extern main
        global _start
_start:
	xor	rbp, rbp
	pop	rdi			; argc
	mov	rsi, rsp		; argv
	push	rdi
	lea	rdx, [rsi + rdi * 8 + 8]; envp
	and	rsp, -16		; align rsp

	call	main
	mov	rdi, rax
%ifdef Darwin
	mov     rax,0x2000001           ; OSX _exit
%else
	mov     rax,60                  ; Linux _exit
%endif
	syscall
