; macOS syscalls
;default rel            ; default generate rip-relative code

        SECTION .data
        global  errno:object
errno:  dd      0

        SECTION .text
        global _exit:function
_exit:
	mov	rax, 1
__syscall:
        mov     r10, rcx
        or      rax,0x2000000
        clc
        syscall
        jnc     done            ; returns carry and positive errno on error
        mov     [errno], eax
        mov     rax, -1
done:
        ret

global fork:function
global vfork:function
vfork:                  ; vfork - FIXME actually fork
fork:
	mov	rax,0x2000002   ; fork
	clc
	syscall
	jnc	forkok
	mov	[errno], eax
	mov	rax, -1
	ret
forkok:
	or	edx,edx         ; EDX zero in parent
	jz	parent
	xor	rax,rax         ; child
parent:
	ret

global read:function
read:
	mov	eax, 3
	jmp	__syscall

global write:function
write:
	mov	eax, 4
	jmp	__syscall

global open:function
open:
	mov	eax, 5
	jmp	__syscall

global close:function
close:
	mov	eax, 6
	jmp	__syscall

global waitpid:function
waitpid:
	xor	rcx, rcx
	mov	eax, 7          ; wait4
	jmp	__syscall

;global creat:function
;creat:
	;mov	eax, 85
	;jmp	__syscall

global link:function
link:
	mov	eax, 9
	jmp	__syscall

global unlink:function
unlink:
	mov	eax, 10
	jmp	__syscall

global _execve:function
_execve:
	mov	eax, 59
	jmp	__syscall

global chdir:function
chdir:
	mov	eax, 12
	jmp	__syscall

;global time:function
;time:
	;mov	eax, 201
	;jmp	__syscall

global mknod:function
mknod:
	mov	eax, 14
	jmp	__syscall

global chmod:function
chmod:
	mov	eax, 15
	jmp	__syscall

global lseek:function
lseek:
	mov	eax, 199
	jmp	__syscall

global getpid:function
getpid:
	mov	eax, 20
	jmp	__syscall

global mount:function
mount:
	mov	eax, 167
	jmp	__syscall

global umount:function
umount:
	xor	rsi, rsi
	mov	eax, 159
	jmp	__syscall

global setuid:function
setuid:
	mov	eax, 23
	jmp	__syscall

global getuid:function
getuid:
	mov	eax, 24
	jmp	__syscall

global utimes:function
utimes:
	mov	eax, 138
	jmp	__syscall

global access:function
access:
	mov	eax, 33
	jmp	__syscall

global sync:function
sync:
	mov	eax, 36
	jmp	__syscall

global kill:function
kill:
	mov	eax, 37
	jmp	__syscall

global mkdir:function
mkdir:
	mov	eax, 136
	jmp	__syscall

global rmdir:function
rmdir:
	mov	eax, 1327
	jmp	__syscall

global dup:function
dup:
	mov	eax, 41
	jmp	__syscall

global pipe:function
pipe:
	mov	eax, 42
	jmp	__syscall

;global brk:function
;brk:
	;mov	eax, 12
	;jmp	__syscall

global setgid:function
setgid:
	mov	eax, 181
	jmp	__syscall

global getgid:function
getgid:
	mov	eax, 47
	jmp	__syscall

global geteuid:function
geteuid:
	mov	eax, 25
	jmp	__syscall

global getegid:function
getegid:
	mov	eax, 43
	jmp	__syscall

global ioctl:function
ioctl:
	mov	eax, 54
	jmp	__syscall

global fcntl:function
fcntl:
	mov	eax, 92
	jmp	__syscall

global dup2:function
dup2:
	mov	eax, 90
	jmp	__syscall

global getppid:function
getppid:
	mov	eax, 39
	jmp	__syscall

global setsid:function
setsid:
	mov	eax, 147
	jmp	__syscall

global gettimeofday:function
gettimeofday:
	mov	eax, 116
	jmp	__syscall

global settimeofday:function
settimeofday:
	mov	eax, 122
	jmp	__syscall

global mmap:function
mmap:
	mov	eax, 197
	jmp	__syscall

global munmap:function
munmap:
	mov	eax, 73
	jmp	__syscall

global stat:function
stat:
	;mov	eax, 188	; stat
	mov	eax, 338	; stat64
	jmp	__syscall

global lstat:function
lstat:
	;mov	eax, 190	; lstat
	mov	eax, 340	; lstat64
	jmp	__syscall

global fstat:function
fstat:
	;mov	eax, 189	; fstat
	mov	eax, 339        ; fstat64
	jmp	__syscall

global getdirentries:function
getdirentries:
	;mov	eax, 196	; getdirentries
	mov	eax, 344	; getdirentries64
	jmp	__syscall

;global clone:function
;clone:
	;mov	eax, 56
	;jmp	__syscall

;global uname:function
;uname:
	;mov	eax, 63
	;jmp	__syscall

global fchdir:function
fchdir:
	mov	eax, 13
	jmp	__syscall

;global nanosleep:function
;nanosleep:
	;mov	eax, 35
	;jmp	__syscall

global poll:function
poll:
	mov	eax, 230
	jmp	__syscall

global chown:function
chown:
	mov	eax, 16
	jmp	__syscall

;global getcwd:function
;getcwd:
	;mov	eax, 79
	;jmp	__syscall

global sigaction:function
sigaction:
	mov	eax, 46
	jmp	__syscall

global sigreturn:function
sigreturn:
	mov	eax, 184
	jmp	__syscall

global fsync:function
fsync:
	mov	eax, 95
	jmp	__syscall

global fdatasync:function
fdatasync:
	mov	eax, 187
	jmp	__syscall

global truncate:function
truncate:
	mov	eax, 200
	jmp	__syscall

global ftruncate:function
ftruncate:
	mov	eax, 201
	jmp	__syscall

global rename:function
rename:
	mov	eax, 128
	jmp	__syscall

global umask:function
umask:
	mov	eax, 60
	jmp	__syscall

global symlink:function
symlink:
	mov	eax, 57
	jmp	__syscall

global readlink:function
readlink:
	mov	eax, 58
	jmp	__syscall
