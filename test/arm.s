.global _start
.extern main
_start:
	mov	fp, #0
	ldr	r0, [sp], #4
	mov	r1, sp
	add	r2, r1, r0, lsl #2
	add	r2, r2, #4
	bl	main
	mov	r7, #1
	swi	#0
