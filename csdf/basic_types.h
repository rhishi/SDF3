/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   basic_types.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   CSDF basic type definitions
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: basic_types.h,v 1.1 2008/03/22 14:24:21 sander Exp $
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

#ifndef CSDF_BASIC_TYPES_H_INCLUDED
#define CSDF_BASIC_TYPES_H_INCLUDED

typedef unsigned short TCnt;
typedef unsigned long TBufSize;
typedef unsigned long long TTime;
typedef double TDtime;

#define TDTIME_MAX      INT_MAX
#define TBUFSIZE_MAX    ULONG_MAX
#define TTIME_MAX       ULONG_LONG_MAX

#endif

