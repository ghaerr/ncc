#ifndef	_TIME_H
#define	_TIME_H

#include <sys/types.h>

struct timespec {
	long tv_sec;
	long tv_nsec;
};

int nanosleep(struct timespec *req, struct timespec *rem);

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
	long tm_gmtoff;
	char *tm_zone;
};

time_t time(time_t *timep);
long strftime(char *s, long len, char *fmt, struct tm *tm);
struct tm *localtime(time_t *timep);
struct tm *gmtime(time_t *timep);

extern long timezone;
extern int _tz_is_set;
void tzset(void);

#define __isleap(year)  \
  ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
void __tm_conv(struct tm *tmbuf, const time_t *timep, time_t offset);
void __asctime(char *buffer, const struct tm *ptm);

#endif
