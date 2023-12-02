# output architecture: x64, x86, arm
OUT = x64

# platform uname for syscalls
PLATFORM=$(shell uname)

CC = ../neatcc/ncc	# the neatcc binary
LD = ../neatld/nld	# the neatld binary

# default assemblers
ASx64 = nasm -f elf64 -d$(PLATFORM)
ASx86 = nasm -f elf
ASarm = neatas

AS = $(AS$(OUT))

all: $(OUT).o \
	b01.x b02.x b03.x b04.x b05.x b06.x b07.x b08.x b09.x b10.x \
	b11.x b12.x b13.x b14.x b15.x b16.x b17.x b18.x b19.x b20.x \
	b21.x b22.x \
	t00.x t01.x t02.x t03.x t04.x t05.x t06.x t07.x \
	t08.x t09.x t0a.x t0b.x t0c.x t0d.x t0e.x t0f.x \
	t10.x t11.x t12.x t13.x t14.x t15.x t16.x t17.x \
	t18.x t19.x t1a.x t1b.x t1c.x t1d.x t1e.x t1f.x \
	t20.x t21.x t22.x t23.x t24.x t25.x t26.x t27.x \
	t28.x t29.x t2a.x t2b.x t2c.x t2d.x t2e.x t2f.x \
	t30.x t31.x t32.x t33.x t34.x t35.x t36.x t37.x \
	t38.x t39.x t3a.x t3b.x t3c.x t3d.x t3e.x t3f.x \
	t40.x t41.x t42.x t43.x t44.x t45.x t46.x t47.x \
	t48.x t49.x t4a.x t4b.x t4c.x t4d.x t4e.x t4f.x \
	t50.x t51.x t52.x t53.x t54.x t55.x t56.x t57.x \
	t58.x t59.x t5a.x t5b.x t5c.x t5d.x t5e.x t5f.x \
	t60.x t61.x t62.x t63.x t64.x t65.x t66.x t67.x \
	t68.x t69.x t6a.x t6b.x t6c.x t6d.x t6e.x t6f.x \
	t70.x t71.x t72.x t73.x t74.x t75.x t76.x t77.x \
	t78.x t79.x t7a.x t7b.x t7c.x t7d.x t7e.x t7f.x \
	t80.x t81.x t82.x t83.x t84.x t85.x t86.x t87.x \
	t88.x t89.x t8a.x t8b.x t8c.x t8d.x t8e.x t8f.x \
	t90.x t91.x t92.x t93.x t94.x t95.x t96.x t97.x \
	t98.x t99.x
	./test.sh

.SECONDARY:
%.o: %.s
	$(AS) $^ -o $@
%.o: %.c
	$(CC) -c $<
%.x: %.o $(OUT).o
	$(LD) -o $@ $^
clean:
	rm -f *.o t??.x b??.x
