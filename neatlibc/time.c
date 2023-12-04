#include <stddef.h>
#include <sys/time.h>
#include <utime.h>

#ifdef __APPLE__
time_t time(time_t *tloc)
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) < 0)
        return -1;

    if (tloc)
        *tloc = tv.tv_sec;

    return tv.tv_sec;
}

int utime(char *path, struct utimbuf *times)
{
    struct timeval tv[2], *tvp;

    if (times) {
        tv[0].tv_sec = times->actime;
        tv[1].tv_sec = times->modtime;
        tv[0].tv_usec = tv[1].tv_usec = 0;
        tvp = tv;
    } else
        tvp = NULL;
    return utimes(path, tvp);
}
#endif
