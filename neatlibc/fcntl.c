#include <fcntl.h>

#ifdef __APPLE__
int creat(const char *path, int mode)
{
    return open(path, O_CREAT | O_TRUNC | O_WRONLY, mode);
}
#endif
