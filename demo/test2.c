#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

//int errno;
#if 0
int strlen(char *p)
{
    int n = 0;
    while (*p++)
        n++;
    return n;
}
#endif

static char *Utox(char *p, unsigned long x) {
  int i;
  if (x) {
#if 0
    *p++ = '0';
    *p++ = 'x';
    i = (__builtin_clzl(x) ^ (sizeof(long) * 8 - 1)) + 1;
    i = (i + 3) & -4;
#else
    i = 64;
#endif
    do {
      if (i == 32) *p++ = '_';
      *p++ = "0123456789abcdef"[(x >> (i -= 4)) & 15];
    } while (i);
  } else {
    *p++ = '0';
  }
  *p = 0;
  return p;
}

void foo(char *a, char *b, char *c, char *d, char *e)
{
    //foo("a", "b", "c", "d", "e");
    //foo(a, b, c, d, e);
    int (*fn)() = strlen;
    fn();
}

int main(int argc, char **argv, char **envp)
{
#if 0
    int i;
    i = open("sdf", 0);
    printf("open %d errno %d\n", i, errno);
    printf("%s\n", "hello world!!");
    printf("main = %lx\n", main);
    ttyname(1);
    for (i=0; i < argc; i++)
        printf("'%lx,%s' ", argv[i], argv[i]);
        //printf("'%lx' ", argv + i);
    printf("\n");
    for (i=0; i<32; i++) printf("malloc %lx\n", malloc(i & 1? 2048: 8192));
    //while (*envp) printf("'%s'\n", *envp++);
    printf("%lx\n", 0xFFFF88881234abcd);
    long x = 0xFFFF88881234abcd;
    char buf[21];
    Utox(buf, x);
    write(1, buf, strlen(buf));
    write(1, "\n", 1);
    printf("argc = %d @ 0x%lx\n", argc, &argc);
	write(1, "NEATCC!\n", 8);
	//printf("NEATCC!\n");
#endif
#if 1
    //execle("../ldelf/ldelf", "../ldelf/ldelf", "./a.out", "a", "b", "C", 0, envp);
    if (fork() == 0) {
        execle("/bin/echo", "/bin/echo", "a", "b", "C", 0, envp);
        printf("EXEC failed\n");
        _exit(0);
    }
    int st;
    wait(&st);
    printf("END\n");
#endif
    char buf[512];
    struct stat sbuf;
    char buf2[512];
    int fd = open("./a.out", 0);
    int i = fstat(fd, &sbuf);
    printf("stat %d\n", i);
    printf("st_dev %lx, st_ino %ld st_mode %o\n",
        sbuf.st_dev, sbuf.st_ino, sbuf.st_mode);
    printf("st_nlink %d uid %d gid %d rdev %x\n", sbuf.st_nlink, sbuf.st_uid, sbuf.st_gid,
        sbuf.st_rdev);
    printf("st_size %ld st_blocks %ld st_blksize %d \n", sbuf.st_size, sbuf.st_blocks,
        sbuf.st_blksize);
    ttyname(1);
    return 0;
}
