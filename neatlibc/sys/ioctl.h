
#ifdef __APPLE__
/* sys/ioccom.h */
#define IOCPARM_MASK    0x1fff          /* parameter length, at most 13 bits */
#define IOC_VOID        0x20000000
#define IOC_OUT         0x40000000      /* copy parameters out */
#define IOC_IN          0x80000000      /* copy parameters in */
#define IOC_INOUT       (IOC_IN|IOC_OUT)/* copy paramters in and out */
#define _IOC(inout, group, num, len) \
        (inout | ((len & IOCPARM_MASK) << 16) | ((group) << 8) | (num))
#define _IO(g, n)       _IOC(IOC_VOID, (g), (n), 0)
#define _IOR(g, n, t)   _IOC(IOC_OUT, (g), (n), sizeof(t))
#define _IOW(g, n, t)   _IOC(IOC_IN,  (g), (n), sizeof(t))
#define _IOWR(g, n, t)  _IOC(IOC_INOUT,   (g), (n), sizeof(t))

/* sys/ttycom.h */
#define TIOCGETA        _IOR('t', 19, struct termios) /* get termios struct */
#define TIOCSETA        _IOW('t', 20, struct termios) /* set termios struct */
#define TIOCSETAW       _IOW('t', 21, struct termios) /* drain output, set */
#define TIOCSETAF       _IOW('t', 22, struct termios) /* drn out, fls in, set */

/* compatibility defines */
#define TCGETS          TIOCGETA
#define TCSETS          TIOCSETA
#define TCSETSW         TIOCSETAW
#define TCSETSF         TIOCSETAF

#else

#define TCGETS		0x5401
#define TCSETS		0x5402
#define TCSETSW		0x5403
#define TCSETSF		0x5404
#define TCGETA		0x5405
#define TCSETA		0x5406
#define TCSETAW		0x5407
#define TCSETAF		0x5408
#define TCSBRK		0x5409
#define TCXONC		0x540A
#define TCFLSH		0x540B
#define TIOCEXCL	0x540C
#define TIOCNXCL	0x540D
#define TIOCSCTTY	0x540E
#define TIOCGPGRP	0x540F
#define TIOCSPGRP	0x5410
#define TIOCOUTQ	0x5411
#define TIOCSTI		0x5412
#define TIOCGWINSZ	0x5413
#define TIOCSWINSZ	0x5414
#define TIOCMGET	0x5415
#define TIOCMBIS	0x5416
#define TIOCMBIC	0x5417
#define TIOCMSET	0x5418
#define TIOCGSOFTCAR	0x5419
#define TIOCSSOFTCAR	0x541A
#define FIONREAD	0x541B
#define TIOCINQ		FIONREAD
#define TIOCLINUX	0x541C
#define TIOCCONS	0x541D
#define TIOCGSERIAL	0x541E
#define TIOCSSERIAL	0x541F
#define TIOCPKT		0x5420
#define FIONBIO		0x5421
#define TIOCNOTTY	0x5422
#define TIOCSETD	0x5423
#define TIOCGETD	0x5424
#define TCSBRKP		0x5425
#define TIOCTTYGSTRUCT	0x5426
#define TIOCSBRK	0x5427
#define TIOCCBRK	0x5428
#define TIOCGSID	0x5429
#define TIOCGPTN	0x80045430
#define TIOCSPTLCK	0x40045431

/* socket-level I/O control calls. */
#define FIOSETOWN 	0x8901
#define SIOCSPGRP	0x8902
#define FIOGETOWN	0x8903
#define SIOCGPGRP	0x8904
#define SIOCATMARK	0x8905
#define SIOCGSTAMP	0x8906

#endif

int ioctl(int fd, int cmd, ...);
