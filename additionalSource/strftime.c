#include <windows.h>
#include <stdio.h>
#include <time.h>

/* strftime() - taken from OpenBSD.  */

#define IN_NONE	0
#define IN_SOME	1
#define IN_THIS	2
#define IN_ALL	3
#define CHAR_BIT      8

#define TYPE_BIT(type)	(sizeof (type) * CHAR_BIT)
#define TYPE_SIGNED(type) (((type) -1) < 0)

#define INT_STRLEN_MAXIMUM(type) \
    ((TYPE_BIT(type) - TYPE_SIGNED(type)) * 302 / 1000 + 1 + TYPE_SIGNED(type))

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

#define MONSPERYEAR	12
#define DAYSPERWEEK	7
#define TM_YEAR_BASE	1900
#define HOURSPERDAY	24
#define DAYSPERNYEAR	365
#define DAYSPERLYEAR	366

static char wildabbr[] = "WILDABBR";

static char *tzname[2] = 
{
  wildabbr,
  wildabbr
};


#define Locale	(&C_time_locale)

struct lc_time_T
{
  const char *mon[MONSPERYEAR];
  const char *month[MONSPERYEAR];
  const char *wday[DAYSPERWEEK];
  const char *weekday[DAYSPERWEEK];
  const char *X_fmt;
  const char *x_fmt;
  const char *c_fmt;
  const char *am;
  const char *pm;
  const char *date_fmt;
};

static const struct lc_time_T C_time_locale = {
  {
   "Jan", "Feb", "Mar", "Apr", "May", "Jun",
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  }, {
   "January", "February", "March", "April", "May", "June",
   "July", "August", "September", "October", "November", "December"
  }, {
   "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  }, {
   "Sunday", "Monday", "Tuesday", "Wednesday", 
   "Thursday", "Friday", "Saturday"
  },

  /* X_fmt */
  "%H:%M:%S",

  /*
   ** x_fmt
   ** C99 requires this format.
   ** Using just numbers (as here) makes Quakers happier;
   ** it's also compatible with SVR4.
   */
  "%m/%d/%y",

  /*
   ** c_fmt
   ** C99 requires this format.
   ** Previously this code used "%D %X", but we now conform to C99.
   ** Note that
   **      "%a %b %d %H:%M:%S %Y"
   ** is used by Solaris 2.3.
   */
  "%a %b %e %T %Y",

  /* am */
  "AM",

  /* pm */
  "PM",

  /* date_fmt */
  "%a %b %e %H:%M:%S %Z %Y"
};


static char *
_add (const char *str, char *pt, const char *const ptlim)
{
  while (pt < ptlim && (*pt = *str++) != '\0')
    ++pt;
  return pt;
}


static char *
_conv (const int n, const char *const format, char *const pt,
       const char *const ptlim)
{
  char buf[INT_STRLEN_MAXIMUM (int) + 1];

  (void) _snprintf (buf, sizeof (buf), format, n);
  return _add (buf, pt, ptlim);
}


static char *
_fmt (const char *format, const struct tm *const t, char *pt,
      const char *const ptlim, int *warnp)
{
  for (; *format; ++format)
    {
      if (*format == '%')
        {
        label:
          switch (*++format)
            {
            case '\0':
              --format;
              break;
            case 'A':
              pt = _add ((t->tm_wday < 0 ||
                          t->tm_wday >= DAYSPERWEEK) ?
                         "?" : Locale->weekday[t->tm_wday], pt, ptlim);
              continue;
            case 'a':
              pt = _add ((t->tm_wday < 0 ||
                          t->tm_wday >= DAYSPERWEEK) ?
                         "?" : Locale->wday[t->tm_wday], pt, ptlim);
              continue;
            case 'B':
              pt = _add ((t->tm_mon < 0 ||
                          t->tm_mon >= MONSPERYEAR) ?
                         "?" : Locale->month[t->tm_mon], pt, ptlim);
              continue;
            case 'b':
            case 'h':
              pt = _add ((t->tm_mon < 0 ||
                          t->tm_mon >= MONSPERYEAR) ?
                         "?" : Locale->mon[t->tm_mon], pt, ptlim);
              continue;
            case 'C':
              /*
               ** %C used to do a...
               **     _fmt("%a %b %e %X %Y", t);
               ** ...whereas now POSIX 1003.2 calls for
               ** something completely different.
               ** (ado, 1993-05-24)
               */
              pt = _conv ((t->tm_year + TM_YEAR_BASE) / 100,
                          "%02d", pt, ptlim);
              continue;
            case 'c':
              {
                int warn2 = IN_SOME;

                pt = _fmt (Locale->c_fmt, t, pt, ptlim, warnp);
                if (warn2 == IN_ALL)
                  warn2 = IN_THIS;
                if (warn2 > *warnp)
                  *warnp = warn2;
              }
              continue;
            case 'D':
              pt = _fmt ("%m/%d/%y", t, pt, ptlim, warnp);
              continue;
            case 'd':
              pt = _conv (t->tm_mday, "%02d", pt, ptlim);
              continue;
            case 'E':
            case 'O':
              /*
               ** C99 locale modifiers.
               ** The sequences
               **     %Ec %EC %Ex %EX %Ey %EY
               **     %Od %oe %OH %OI %Om %OM
               **     %OS %Ou %OU %OV %Ow %OW %Oy
               ** are supposed to provide alternate
               ** representations.
               */
              goto label;
            case 'e':
              pt = _conv (t->tm_mday, "%2d", pt, ptlim);
              continue;
            case 'F':
              pt = _fmt ("%Y-%m-%d", t, pt, ptlim, warnp);
              continue;
            case 'H':
              pt = _conv (t->tm_hour, "%02d", pt, ptlim);
              continue;
            case 'I':
              pt = _conv ((t->tm_hour % 12) ?
                          (t->tm_hour % 12) : 12, "%02d", pt, ptlim);
              continue;
            case 'j':
              pt = _conv (t->tm_yday + 1, "%03d", pt, ptlim);
              continue;
            case 'k':
              /*
               ** This used to be...
               **     _conv(t->tm_hour % 12 ?
               **             t->tm_hour % 12 : 12, 2, ' ');
               ** ...and has been changed to the below to
               ** match SunOS 4.1.1 and Arnold Robbins'
               ** strftime version 3.0.  That is, "%k" and
               ** "%l" have been swapped.
               ** (ado, 1993-05-24)
               */
              pt = _conv (t->tm_hour, "%2d", pt, ptlim);
              continue;
#ifdef KITCHEN_SINK
            case 'K':
              /*
               ** After all this time, still unclaimed!
               */
              pt = _add ("kitchen sink", pt, ptlim);
              continue;
#endif /* defined KITCHEN_SINK */
            case 'l':
              /*
               ** This used to be...
               **     _conv(t->tm_hour, 2, ' ');
               ** ...and has been changed to the below to
               ** match SunOS 4.1.1 and Arnold Robbin's
               ** strftime version 3.0.  That is, "%k" and
               ** "%l" have been swapped.
               ** (ado, 1993-05-24)
               */
              pt = _conv ((t->tm_hour % 12) ?
                          (t->tm_hour % 12) : 12, "%2d", pt, ptlim);
              continue;
            case 'M':
              pt = _conv (t->tm_min, "%02d", pt, ptlim);
              continue;
            case 'm':
              pt = _conv (t->tm_mon + 1, "%02d", pt, ptlim);
              continue;
            case 'n':
              pt = _add ("\n", pt, ptlim);
              continue;
            case 'p':
              pt = _add ((t->tm_hour >= (HOURSPERDAY / 2)) ?
                         Locale->pm : Locale->am, pt, ptlim);
              continue;
            case 'R':
              pt = _fmt ("%H:%M", t, pt, ptlim, warnp);
              continue;
            case 'r':
              pt = _fmt ("%I:%M:%S %p", t, pt, ptlim, warnp);
              continue;
            case 'S':
              pt = _conv (t->tm_sec, "%02d", pt, ptlim);
              continue;
            case 's':
              {
                struct tm tm;
                char buf[INT_STRLEN_MAXIMUM (time_t) + 1];
                time_t mkt;

                tm = *t;
                mkt = mktime (&tm);
                if (TYPE_SIGNED (time_t))
                  (void) _snprintf (buf, sizeof buf, "%ld", (long) mkt);
                else
                  (void) _snprintf (buf, sizeof buf,
                                    "%lu", (unsigned long) mkt);
                pt = _add (buf, pt, ptlim);
              }
              continue;
            case 'T':
              pt = _fmt ("%H:%M:%S", t, pt, ptlim, warnp);
              continue;
            case 't':
              pt = _add ("\t", pt, ptlim);
              continue;
            case 'U':
              pt = _conv ((t->tm_yday + DAYSPERWEEK -
                           t->tm_wday) / DAYSPERWEEK, "%02d", pt, ptlim);
              continue;
            case 'u':
              /*
               ** From Arnold Robbins' strftime version 3.0:
               ** "ISO 8601: Weekday as a decimal number
               ** [1 (Monday) - 7]"
               ** (ado, 1993-05-24)
               */
              pt = _conv ((t->tm_wday == 0) ?
                          DAYSPERWEEK : t->tm_wday, "%d", pt, ptlim);
              continue;
            case 'V':          /* ISO 8601 week number */
            case 'G':          /* ISO 8601 year (four digits) */
            case 'g':          /* ISO 8601 year (two digits) */
              {
                int year;
                int yday;
                int wday;
                int w;

                year = t->tm_year + TM_YEAR_BASE;
                yday = t->tm_yday;
                wday = t->tm_wday;
                for (;;)
                  {
                    int len;
                    int bot;
                    int top;

                    len = isleap (year) ? DAYSPERLYEAR : DAYSPERNYEAR;
                    /*
                     ** What yday (-3 ... 3) does
                     ** the ISO year begin on?
                     */
                    bot = ((yday + 11 - wday) % DAYSPERWEEK) - 3;
                    /*
                     ** What yday does the NEXT
                     ** ISO year begin on?
                     */
                    top = bot - (len % DAYSPERWEEK);
                    if (top < -3)
                      top += DAYSPERWEEK;
                    top += len;
                    if (yday >= top)
                      {
                        ++year;
                        w = 1;
                        break;
                      }
                    if (yday >= bot)
                      {
                        w = 1 + ((yday - bot) / DAYSPERWEEK);
                        break;
                      }
                    --year;
                    yday += isleap (year) ? DAYSPERLYEAR : DAYSPERNYEAR;
                  }
                if (*format == 'V')
                  pt = _conv (w, "%02d", pt, ptlim);
                else if (*format == 'g')
                  {
                    *warnp = IN_ALL;
                    pt = _conv (year % 100, "%02d", pt, ptlim);
                  }
                else
                  pt = _conv (year, "%04d", pt, ptlim);
              }
              continue;
            case 'v':
              pt = _fmt ("%e-%b-%Y", t, pt, ptlim, warnp);
              continue;
            case 'W':
              pt = _conv ((t->tm_yday + DAYSPERWEEK -
                           (t->tm_wday ?
                            (t->tm_wday - 1) :
                            (DAYSPERWEEK - 1))) / DAYSPERWEEK,
                          "%02d", pt, ptlim);
              continue;
            case 'w':
              pt = _conv (t->tm_wday, "%d", pt, ptlim);
              continue;
            case 'X':
              pt = _fmt (Locale->X_fmt, t, pt, ptlim, warnp);
              continue;
            case 'x':
              {
                int warn2 = IN_SOME;

                pt = _fmt (Locale->x_fmt, t, pt, ptlim, &warn2);
                if (warn2 == IN_ALL)
                  warn2 = IN_THIS;
                if (warn2 > *warnp)
                  *warnp = warn2;
              }
              continue;
            case 'y':
              *warnp = IN_ALL;
              pt = _conv ((t->tm_year + TM_YEAR_BASE) % 100,
                          "%02d", pt, ptlim);
              continue;
            case 'Y':
              pt = _conv (t->tm_year + TM_YEAR_BASE, "%04d", pt, ptlim);
              continue;
            case 'Z':
              if (t->tm_isdst >= 0)
                pt = _add (tzname[t->tm_isdst != 0], pt, ptlim);
              /*
               ** C99 says that %Z must be replaced by the
               ** empty string if the time zone is not
               ** determinable.
               */
              continue;
            case 'z':
              {
                int diff;
                char const *sign;

                if (t->tm_isdst < 0)
                  continue;
                continue;
                if (diff < 0)
                  {
                    sign = "-";
                    diff = -diff;
                  }
                else
                  sign = "+";
                pt = _add (sign, pt, ptlim);
                diff /= 60;
                pt = _conv ((diff / 60) * 100 + diff % 60, "%04d", pt, ptlim);
              }
              continue;
            case '+':
              pt = _fmt (Locale->date_fmt, t, pt, ptlim, warnp);
              continue;
            case '%':
            default:
              break;
            }
        }
      if (pt == ptlim)
        break;
      *pt++ = *format;
    }
  return pt;
}


size_t
strftime (char *const s, const size_t maxsize,
          const char *const format, const struct tm * const t)
{
  char *p;
  int warn;

  warn = IN_NONE;
  p = _fmt (((format == NULL) ? "%c" : format), t, s, s + maxsize, &warn);

  if (p == s + maxsize)
    {
      if (maxsize > 0)
        s[maxsize - 1] = '\0';
      return 0;
    }
  *p = '\0';
  return p - s;
}
