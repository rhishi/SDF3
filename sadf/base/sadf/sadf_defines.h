/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_defines.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Definitions
 *
 *  History         :
 *      29-08-06    :   Initial version.
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

#ifndef SADF_DEFINES_H_INCLUDED
#define SADF_DEFINES_H_INCLUDED

// Include basic type definitions

#include "../../../base/base.h"

// Constants as macros - general

#define SADF_UNDEFINED		((CId)(-1))
#define SADF_MAX_DOUBLE		DBL_MAX

// Constants as macros - channels (SADF)

#define SADF_UNBOUNDED		((CId)(-1))

#define SADF_DATA_CHANNEL 	0
#define SADF_CONTROL_CHANNEL 	1

// Constants as macros - processes (SADF)

#define SADF_KERNEL 		0
#define SADF_DETECTOR 		1

// Constants as macros - transitions (TPS)

#define SADF_TIME_STEP 0
#define SADF_START_STEP 1
#define SADF_END_STEP 2
#define SADF_CONTROL_STEP 3
#define SADF_DETECT_STEP 4

#endif
