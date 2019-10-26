format ELF

extrn main
public _start
_start:
	xor	ebp, ebp
	xor	eax, eax
	push	6
	call	main
	pop	edx
	mov	ebx, eax
	mov	eax, 1
	int	0x80
