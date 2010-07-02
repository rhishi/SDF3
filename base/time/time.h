/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   time.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 25, 2005
 *
 *  Function        :   Measure elapsed time for program fragments.
 *
 *  History         :
 *      25-07-05    :   Initial version.
 *
 * $Id: time.h,v 1.1.1.1 2007/10/02 10:59:48 sander Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 * In other words, you are welcome to use, share and improve this program.
 * You are forbidden to forbid anyone else to use, share and improve
 * what you give them.   Happy coding!
 */

#ifndef BASE_TIME_TIME_H_INCLUDED
#define BASE_TIME_TIME_H_INCLUDED

#include "../basic_types.h"
#include "../string/cstring.h"
#include <time.h>
#include <sys/resource.h>

typedef struct _CTimer
{
    struct rusage rStart;
    struct rusage rEnd;
    struct timeval time;
} CTimer;

/**
 * startTimer ()
 * The function start measuring the elapsed user and system time 
 * from this point onwards.
 */
static inline
void startTimer(CTimer *t)
{
    getrusage(RUSAGE_SELF, &(t->rStart));
}

/**
 * stopTimer ()
 * The function stops measuring the elapsed user and system time and
 * it computes the total elapsed time. This time is accesible via 
 * t->time (struct timeval).
 */
static inline
void stopTimer(CTimer *t)
{
    getrusage(RUSAGE_SELF, &(t->rEnd));

    // Calculate elapsed time (user + system)
    t->time.tv_sec = t->rEnd.ru_utime.tv_sec - t->rStart.ru_utime.tv_sec
                        + t->rEnd.ru_stime.tv_sec - t->rEnd.ru_stime.tv_sec;
    t->time.tv_usec = t->rEnd.ru_utime.tv_usec - t->rStart.ru_utime.tv_usec 
                        + t->rEnd.ru_stime.tv_usec - t->rStart.ru_stime.tv_usec;

    if (t->time.tv_sec >= 1e6)
    {
        t->time.tv_sec = t->time.tv_sec + t->time.tv_usec/long(1e6);
        t->time.tv_usec = t->time.tv_usec%long(1e6);
    }
}

/**
 * printTimer ()
 * The function prints the value of the timer in milliseconds to the supplied
 * output stream.
 */
static inline
void printTimer(ostream &out, CTimer *t)
{
    double s = (t->time.tv_sec * (double)(1000)) 
                    + (t->time.tv_usec / (double)(1000));
    
    out << s << "ms";
}

/**
 * printTimer ()
 * The function returns the value of the timer in milliseconds as a string.
 */
static inline
CString printTimer(CTimer *t)
{
    double s = (t->time.tv_sec * (double)(1000)) 
                    + (t->time.tv_usec / (double)(1000));
    
    return (CString(s) + CString("ms"));
}

#endif
