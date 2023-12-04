/* This is adapted from glibc */
/* Copyright (C) 1991, 1993 Free Software Foundation, Inc */

#define SECS_PER_HOUR 3600L
#define SECS_PER_DAY  86400L

#include <time.h>

static const char __mon_lengths[2][12] =
  {
    /* Normal years.  */
    { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
    /* Leap years.  */
    { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
  };


void
__tm_conv(struct tm *tmbuf, const time_t *t, time_t offset)
{
  long days, rem;
  int yday;
  register int y;
  register const char *ip;

  days = *t / SECS_PER_DAY;
  rem = *t - days * SECS_PER_DAY;
  rem += offset;
  while (rem < 0)
    {
      rem += SECS_PER_DAY;
      --days;
    }
  while (rem >= SECS_PER_DAY)
    {
      rem -= SECS_PER_DAY;
      ++days;
    }
  tmbuf->tm_hour = rem / SECS_PER_HOUR;
  rem -= tmbuf->tm_hour * SECS_PER_HOUR;
  tmbuf->tm_min = rem / 60;
  tmbuf->tm_sec = rem - tmbuf->tm_min * 60;
  /* January 1, 1970 was a Thursday.  */
  tmbuf->tm_wday = (4 + days) % 7;
  if (tmbuf->tm_wday < 0)
    tmbuf->tm_wday += 7;
  y = 1970;
  while (days >= (rem = __isleap(y) ? 366 : 365))
    {
      ++y;
      days -= rem;
    }
  while (days < 0)
    {
      --y;
      days += __isleap(y) ? 366 : 365;
    }
  yday = days;
  tmbuf->tm_year = y - 1900;
  tmbuf->tm_yday = yday;
  ip = __mon_lengths[__isleap(y)];
  for (y = 0; yday >= ip[y]; ++y)
    yday -= ip[y];
  tmbuf->tm_mon = y;
  tmbuf->tm_mday = yday + 1;
  tmbuf->tm_isdst = -1;
}
