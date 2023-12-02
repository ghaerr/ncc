#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __APPLE__
/* getdirentries file types */
#define	DT_UNKNOWN	 0
#define	DT_FIFO		 1
#define	DT_CHR		 2
#define	DT_DIR		 4
#define	DT_BLK		 6
#define	DT_REG		 8
#define	DT_LNK		10
#define	DT_SOCK		12
#define	DT_WHT		14

/* opendir flags */
#define DTF_HIDEW       0x0001	/* hide whiteout entries */
#define DTF_NODUP       0x0002	/* don't return duplicate names */
#define DTF_REWIND      0x0004	/* rewind after reading union stack */
#define __DTF_READALL   0x0008	/* everything has been read */

struct __dirent_dir {
    int fd;                 /* file descriptor associated with directory */
    int dd_loc;             /* offset in current buffer */
    int dd_size;            /* amount of data returned by getdirentries */
    long dd_seek;           /* magic cookie returned by getdirentries */
    int dd_flags;           /* flags for readdir (UNUSED) */
    char dd_buf[2048];
};
#else
struct __dirent_dir {
	int fd;
	int buf_pos;
	int buf_end;
	char buf[2048];
};
#endif

DIR *opendir(char *path)
{
	DIR *dir;
	int fd;
	if ((fd = open(path, O_RDONLY | O_DIRECTORY)) < 0)
		return NULL;
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
	if (!(dir = malloc(sizeof(*dir)))) {
		close(fd);
		return NULL;
	}
	memset(dir, 0, sizeof(*dir));
	dir->fd = fd;
	return dir;
}

int closedir(DIR *dir)
{
	int ret;
	ret = close(dir->fd);
	free(dir);
	return ret;
}

#ifdef __APPLE__
int getdirentries(int fd, char *buf, int nbytes, long *basep);

struct dirent * readdir(DIR *dirp)
{
    struct dirent *dp;

    for (;;) {
        if (dirp->dd_loc >= dirp->dd_size) {
            /*if (dirp->dd_flags & __DTF_READALL)
                return (NULL);*/
            dirp->dd_loc = 0;
        }
        if (dirp->dd_loc == 0 /*&& !(dirp->dd_flags & __DTF_READALL)*/) {
            dirp->dd_size = getdirentries(dirp->fd,
                dirp->dd_buf, sizeof(dirp->dd_buf), &dirp->dd_seek);
            if (dirp->dd_size <= 0)
                return NULL;
        }
        dp = (struct dirent *)(dirp->dd_buf + dirp->dd_loc);
        if ((int)dp & 03)       /* bogus pointer check */
            return (NULL);
        if (dp->d_reclen <= 0 || dp->d_reclen > sizeof(dirp->dd_buf) + 1 - dirp->dd_loc)
            return (NULL);
        dirp->dd_loc += dp->d_reclen;
        if (dp->d_ino == 0)
            continue;
        /*if (dp->d_type == DT_WHT && (dirp->dd_flags & DTF_HIDEW))
            continue;*/
        return dp;
    }
}
#else
int getdents(int fd, struct dirent *de, size_t len);

struct dirent *readdir(DIR *dir)
{
	struct dirent *de;
	int len;
	if (dir->buf_pos >= dir->buf_end) {
		len = getdents(dir->fd, (void *) dir->buf, sizeof(dir->buf));
		if (len <= 0)
			return NULL;
		dir->buf_pos = 0;
		dir->buf_end = len;
	}
	de = (void *) (dir->buf + dir->buf_pos);
	dir->buf_pos += de->d_reclen;
	return de;
}
#endif
