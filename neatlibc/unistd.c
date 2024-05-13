#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#ifndef __APPLE__
int sleep(int n)
{
	struct timespec req = {n, 0};
	struct timespec rem;
	if (nanosleep(&req, &rem))
		return rem.tv_sec;
	return 0;
}
#endif

#define EXECARGS	(1 << 7)

int _execve(char *path, char *argv[], char *envp[]);

int execve(char *path, char *argv[], char *envp[])
{
#ifdef __APPLE__
    int fd;
    char *argv2[EXECARGS];

    fd = open(path, 0);
    if (fd >= 0) {
        char **p = argv, **q = argv2;
        char buf[2];
        if (read(fd, buf, 2) == 2 && buf[0] == 0x7f && buf[1] == 0x45) {
            path = LOADER;              //FIXME remove hard path
            printf("+elf %s ", path);
            *q++ = path;
            while ((*q++ = *p++) != 0)
                printf("%s ", q[-1]);
            printf("\n");
            argv = argv2;
        }
        close(fd);
    }
#endif
    return _execve(path, argv, envp);
}

int execle(char *path, ...)
{
	va_list ap;
	char **envp;
	int argc = 0;
	char *argv[EXECARGS];
	va_start(ap, path);
	while (argc + 1 < EXECARGS && (argv[argc] = va_arg(ap, char *)))
		argc++;
	envp = va_arg(ap, char **);
	va_end(ap);
	argv[argc] = NULL;
	execve(path, argv, envp);
	return -1;
}

int execl(char *path, ...)
{
	va_list ap;
	int argc = 0;
	char *argv[EXECARGS];
	va_start(ap, path);
	while (argc + 1 < EXECARGS && (argv[argc] = va_arg(ap, char *)))
		argc++;
	va_end(ap);
	argv[argc] = NULL;
	execve(path, argv, environ);
	return -1;
}

int execvp(char *cmd, char *argv[])
{
	char path[512];
	char *p = getenv("PATH");
	if (strchr(cmd, '/'))
		return execve(cmd, argv, environ);
	if (!p)
		p = "/bin";
	while (*p) {
		char *s = path;
		while (*p && *p != ':')
			*s++ = *p++;
		if (s != path)
			*s++ = '/';
		strcpy(s, cmd);
		execve(path, argv, environ);
		if (*p == ':')
			p++;
	}
	return -1;
}

int execv(char *path, char *argv[])
{
	return execve(path, argv, environ);
}

int wait(int *status)
{
	return waitpid(-1, status, 0);
}

long pread(int fd, void *buf, long n, long offset)
{
    long ret, old;
    int e;

    old = lseek(fd, offset, SEEK_CUR);
    if (lseek(fd, offset, SEEK_SET) == -1)
        return -1;
    ret = read(fd, buf, n);
    e = errno;
    lseek(fd, old, SEEK_CUR);
    errno = e;
    return ret;
}

int raise(int sig)
{
	return kill(getpid(), sig);
}

void abort(void)
{
	raise(SIGABRT);
	while (1)
		;
}

#define errmsg(str) write(2, str, sizeof(str) - 1)
#define errstr(str) write(2, str, strlen(str))

void __assertfailed(char *str)
{
    errmsg("assertion \"");
    errstr(str);
    errmsg("\" failed\n");
    abort();
}
