/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   math.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 14, 2005
 *
 *  Function        :   Mathematical functions
 *
 *  History         :
 *      14-07-05    :   Initial version.
 *
 * $Id: cmath.h,v 1.1.1.1 2007/10/02 10:59:48 sander Exp $
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

#ifndef BASE_MATH_CMATH_H
#define BASE_MATH_CMATH_H

#include <cmath>

// Greatest common divisor
template <class T>
T gcd(T a, T b)
{
    T t;
    
    while (b != 0)
    {
        t = b;
        b = a % b;
        a = t;
    }

    return a;
}


// Least common multiple
template <class T>
T lcm(T a, T b)
{
    if (a == 0 || b == 0)
        return 0;
        
    return ((a * b) / gcd(a, b));
}

#endif
