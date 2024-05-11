#ifndef _FCNTL_H
#define _FCNTL_H

#define O_RDONLY        00000
#define O_WRONLY        00001
#define O_RDWR          00002
#define O_ACCMODE       00003

#ifdef __APPLE__
#define O_NONBLOCK      0x0004          /* no delay */
#define O_APPEND        0x0008          /* set append mode */
#define O_CREAT         0x0200          /* create if nonexistant */
#define O_TRUNC         0x0400          /* truncate to zero length */
#define O_EXCL          0x0800          /* error if already exists */
#define O_NOCTTY        0x20000         /* don't assign controlling terminal */
#define O_DIRECTORY     0x100000
#define O_SYMLINK       0x200000        /* allow open of a symlink */
#define O_CLOEXEC       0x1000000       /* implicitly set FD_CLOEXEC */

/* fcntl(2) */
#define F_DUPFD         0               /* duplicate file descriptor */
#define F_GETFD         1               /* get file descriptor flags */
#define F_SETFD         2               /* set file descriptor flags */
#define F_GETFL         3               /* get file status flags */
#define F_SETFL         4               /* set file status flags */
#define F_GETOWN        5               /* get SIGIO/SIGURG proc/pgrp */
#define F_SETOWN        6               /* set SIGIO/SIGURG proc/pgrp */
#define F_GETLK         7               /* get record locking information */
#define F_SETLK         8               /* set record locking information */
#define F_SETLKW        9               /* F_SETLK; wait if blocked */

#define FD_CLOEXEC      1               /* close-on-exec flag */


#else
#define O_CREAT         00100
#define O_EXCL          00200
#define O_NOCTTY        00400
#define O_TRUNC         01000
#define O_APPEND        02000
#define O_NONBLOCK      04000
#define O_SYNC          0010000
#define FASYNC          0020000
#ifdef __arm__
#define O_DIRECTORY     0040000
#define O_NOFOLLOW      0100000
#define O_DIRECT        0200000
#define O_LARGEFILE     0400000
#else
#define O_DIRECT        0040000
#define O_LARGEFILE     0100000
#define O_DIRECTORY     0200000
#define O_NOFOLLOW      0400000
#endif
#define O_NOATIME       001000000

#define F_DUPFD         0
#define F_GETFD         1
#define F_SETFD         2
#define F_GETFL         3
#define F_SETFL         4
#define F_GETLK         5
#define F_SETLK         6
#define F_SETLKW        7
#define F_SETOWN        8
#define F_GETOWN        9
#define F_SETSIG        10
#define F_GETSIG        11

#define FD_CLOEXEC      1

#define F_RDLCK         0
#define F_WRLCK         1
#define F_UNLCK         2

#endif

int open(char *path, int flags, ...);
int creat(char *path, int mode);
int fcntl(int fd, int cmd, ...);

#endif
