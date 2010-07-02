/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   basic_types.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 18, 2002
 *
 *  Function        :   Basic type definitions
 *
 *  History         :
 *      18-06-04    :   Initial version.
 *	    26-08-06    :   Additions by Bart Theelen.
 *
 * $Id: basic_types.h,v 1.7 2008/11/01 15:57:58 sander Exp $
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

#ifndef BASE_BASIC_TYPES_H_INCLUDED
#define BASE_BASIC_TYPES_H_INCLUDED

#include "float.h"

/* STL functionality */
#include <fstream>
#include <iostream>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <queue>
#include <algorithm>

using std::list;
using std::set;
using std::vector;
using std::map;
using std::istream;
using std::ostream;
using std::ifstream;
using std::ofstream;
using std::fstream;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::queue;

/* basic types */
typedef unsigned int        	uint;
typedef std::vector<int>    	v_int;
typedef std::vector<uint>   	v_uint;

/* Id */
typedef uint CId;
typedef unsigned long long  	CSize;

/* Doubles */
typedef double		        	CDouble;

/* Queue */
typedef queue<CId>              CQueue;

#define CSIZE_MAX               ULONG_LONG_MAX
#define CID_MAX                 UINT_MAX

#endif
