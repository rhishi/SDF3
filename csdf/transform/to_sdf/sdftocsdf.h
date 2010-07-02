/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdftocsdf.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   Convert an SDF graph to a CSDF graph
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: sdftocsdf.h,v 1.1.1.1 2007/10/02 10:59:48 sander Exp $
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

#ifndef CSDF_TRNSFORM_TO_SDF_SDFTOCSDF_H_INCLUDED
#define CSDF_TRNSFORM_TO_SDF_SDFTOCSDF_H_INCLUDED

#include "sdf/sdf.h"
#include "../../csdf.h"

/**
 * convertSDFGtoCSDFG ()
 * The function converts an SDFG to an equivalent CSDFG.
 */
TimedCSDFgraph *convertSDFGtoCSDFG(TimedSDFgraph *sdfGraph);

#endif
