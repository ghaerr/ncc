/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:2;tab-width:8;coding:utf-8 -*-│
│vi: set net ft=c ts=2 sts=2 sw=2 fenc=utf-8                                :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2021 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/

/* system call and utility routines for portable static binaries */

#include "../neatlibc/syscalls.h"

#define LINUX   1
#define XNU     8
#define OPENBSD 16
#define FREEBSD 32
#define NETBSD  64

#define SUPPORT_VECTOR    (XNU|LINUX|FREEBSD|OPENBSD|NETBSD)

#define SupportsLinux()   (SUPPORT_VECTOR & LINUX)
#define SupportsXnu()     (SUPPORT_VECTOR & XNU)
#define SupportsFreebsd() (SUPPORT_VECTOR & FREEBSD)
#define SupportsOpenbsd() (SUPPORT_VECTOR & OPENBSD)
#define SupportsNetbsd()  (SUPPORT_VECTOR & NETBSD)

#define IsLinux()   (SupportsLinux() && os == LINUX)
#define IsXnu()     (SupportsXnu() && os == XNU)
#define IsFreebsd() (SupportsFreebsd() && os == FREEBSD)
#define IsOpenbsd() (SupportsOpenbsd() && os == OPENBSD)
#define IsNetbsd()  (SupportsNetbsd() && os == NETBSD)

#ifdef __aarch64__
#define IsAarch64() 1
#else
#define IsAarch64() 0
#endif

#define AT_PAGESZ          6
#define AT_EXECFN_NETBSD   2014

static int os;
static int pagesz;

extern long SystemCall(long rdi, long rsi, long rdx, long rcx_r10, long r8, long r9,
                       long arg7_8rsp, int ax_16rsp);

__attribute__((__noinline__))
static long CallSystem(long arg1, long arg2, long arg3, long arg4, long arg5,
                       long arg6, long arg7, int numba) {
  if (IsXnu()) numba |= 0x2000000;
  return SystemCall(arg1, arg2, arg3, arg4, arg5, arg6, arg7, numba);
}

long *InitAPE(long di, long *sp, char dl) {
  int argc;
  long *auxv, *ap;;
  char **argv, **envp;

  /* detect freebsd */
  if (SupportsXnu() && dl == XNU) {
    os = XNU;
  } else if (SupportsFreebsd() && di) {
    os = FREEBSD;
    sp = (long *)di;
  } else {
    os = 0;
  }

  /* extract arguments */
  argc = *sp;
  argv = (char **)(sp + 1);
  envp = (char **)(sp + 1 + argc + 1);
  auxv = sp + 1 + argc + 1;
  for (;;) {
    if (!*auxv++) {
      break;
    }
  }

  /* detect openbsd */
  if (SupportsOpenbsd() && !os && !auxv[0]) {
    os = OPENBSD;
  }

  /* xnu passes auxv as an array of strings */
  if (os == XNU) {
    *auxv = 0;
  }

  /* detect netbsd and find end of words */
  pagesz = 0;
  for (ap = auxv; ap[0]; ap += 2) {
    if (ap[0] == AT_PAGESZ) {
      pagesz = ap[1];
    } else if (SupportsNetbsd() && !os && ap[0] == AT_EXECFN_NETBSD) {
      os = NETBSD;
    }
  }
  if (!pagesz) {
    pagesz = 4096;
  }

  /* the default operating system */
  if (!os) {
    os = LINUX;
  }
  return sp;
}

long Write(int fd, const void *data, unsigned long size) {
  int numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 64;
    } else {
      numba = 1;
    }
  } else {
    numba = 4;
  }
  return CallSystem(fd, (long)data, size, 0, 0, 0, 0, numba);
}

__attribute__((__noreturn__)) void Exit(int rc) {
  int numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 94;
    } else {
      numba = 60;
    }
  } else {
    numba = 1;
  }
  CallSystem(rc, 0, 0, 0, 0, 0, 0, numba);
  __builtin_unreachable();
}

long StrLen(const char *s) {
  long n = 0;
  while (*s++) ++n;
  return n;
}

char *StrCpy(char *d, const char *s) {
  char *dst = d;
  while ((*d++ = *s++) != '\0')
    ;
  return dst;
}

int StrCmp(const char *l, const char *r) {
  unsigned long i = 0;
  while (l[i] == r[i] && r[i]) ++i;
  return (l[i] & 255) - (r[i] & 255);
}

void Bzero(void *a, unsigned long n) {
  long z;
  volatile char *p;
  char *e;
  p = (char *)a;
  e = (char *)p + n;
  z = 0;
  while (p + sizeof(z) <= e) {
    __builtin_memcpy((void *)p, &z, sizeof(z));
    p += sizeof(z);
  }
  while (p < e) {   /* if not volatile, ___bzero called here on -Os */
    *p++ = 0;
  }
}

void *MemMove(void *a, const void *b, unsigned long n) {
  long w;
  char *d;
  const char *s;
  unsigned long i;
  d = (char *)a;
  s = (const char *)b;
  if (d > s) {
    while (n >= sizeof(w)) {
      n -= sizeof(w);
      __builtin_memcpy(&w, s + n, sizeof(n));
      __builtin_memcpy(d + n, &w, sizeof(n));
    }
    while (n--) {
      d[n] = s[n];
    }
  } else {
    i = 0;
    while (i + sizeof(w) <= n) {
      __builtin_memcpy(&w, s + i, sizeof(i));
      __builtin_memcpy(d + i, &w, sizeof(i));
      i += sizeof(w);
    }
    for (; i < n; ++i) {
      d[i] = s[i];
    }
  }
  return d;
}

long Pread(int fd, void *data, unsigned long size, long off) {
  long numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 0x043;
    } else {
      numba = 0x011;
    }
  } else if (IsXnu()) {
    numba = 0x2000099;
  } else if (IsFreebsd()) {
    numba = 0x1db;
  } else if (IsOpenbsd()) {
    numba = 0x0a9; /* OpenBSD v7.3+ */
  } else if (IsNetbsd()) {
    numba = 0x0ad;
  } else {
    __builtin_unreachable();
  }
  return SystemCall(fd, (long)data, size, off, off, 0, 0, numba);
}

int Open(const char *path, int flags, int mode) {
  if (IsLinux() && IsAarch64()) {
    return SystemCall(-100, (long)path, flags, mode, 0, 0, 0, 56);
  } else {
    return CallSystem((long)path, flags, mode, 0, 0, 0, 0, IsLinux() ? 2 : 5);
  }
}

int Close(int fd) {
  int numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 57;
    } else {
      numba = 3;
    }
  } else {
    numba = 6;
  }
  return CallSystem(fd, 0, 0, 0, 0, 0, 0, numba);
}

long Mmap(void *addr, unsigned long size, int prot, int flags, int fd, long off) {
  long numba;
  if (IsLinux()) {
    if (IsAarch64()) {
      numba = 222;
    } else {
      numba = 9;
    }
  } else if (IsXnu()) {
    numba = 0x2000000 | 197;
  } else if (IsFreebsd()) {
    numba = 477;
  } else if (IsOpenbsd()) {
    numba = 49; /* OpenBSD v7.3+ */
  } else if (IsNetbsd()) {
    numba = 197;
  } else {
    __builtin_unreachable();
  }
  return SystemCall((long)addr, size, prot, flags, fd, off, off, numba);
}

long Read(int fd, void *data, unsigned long size) {
  long numba;
  if (IsLinux()) {
      numba = 0x00;
  } else if (IsXnu()) {
    numba = 0x2000003;
  } else return -1;
  return SystemCall(fd, (long)data, size, 0, 0, 0, 0, numba);
}

extern long SystemCallFork(int numba);

long Fork(void) {
  long numba;
  if (IsLinux()) {
      numba = 57;
  } else if (IsXnu()) {
    numba = 0x2000002;
  } else return -1;
  return SystemCallFork(numba);
}

long WaitPid(int pid, int *status, int options) {
  long numba;
  if (IsLinux()) {
      numba = 61;
  } else if (IsXnu()) {
    numba = 0x2000007;  /* wait4 */
  } else return -1;
  return SystemCall(pid, (long)status, options, 0, 0, 0, 0, numba);
}
