#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

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
            close(fd);
            path = LOADER;              //FIXME remove hard path
            printf("+elf %s ", path);
            *q++ = path;
            while ((*q++ = *p++) != 0)
                printf("%s ", q[-1]);
            printf("\n");
            argv = argv2;
        }
    }
#endif
    return _execve(path, argv, envp);
}

int execle(char *path, ...)
{
	va_list ap;
	char *argv[EXECARGS];
	char **envp;
	int argc = 0;
	va_start(ap, path);
	while (argc + 1 < EXECARGS && (argv[argc] = va_arg(ap, char *)))
		argc++;
	envp = va_arg(ap, char **);
	va_end(ap);
	argv[argc] = NULL;
	execve(path, argv, envp);
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
