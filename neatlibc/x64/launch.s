; void Launch(long rdi, long entry, long sp, long rcx)
; Reset stack pointer and jump to loaded program entry
;
; rdi is passed through as-is
; rsi is address of entrypoint (becomes zero)
; rdx is stack pointer (becomes zero)
; rcx is passed through as-is

        global Launch
Launch:
        xor     r8,r8
        xor     r9,r9
        xor     r10,r10
        xor     r11,r11
        xor     r12,r12
        xor     r13,r13
        xor     r14,r14
        xor     r15,r15
        mov     rsp,rdx
        xor     edx,edx
        push    rsi
        xor     esi,esi
        xor     ebp,ebp
        xor     ebx,ebx
        xor     eax,eax
        ret
