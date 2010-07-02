/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffy.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 25, 2007
 *
 *  Function        :   Buffer sizing for CSDFGs
 *
 *  History         :
 *      25-06-07    :   Initial version.
 *
 * $Id: buffy.h,v 1.1.1.1 2007/10/02 10:59:48 sander Exp $
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

#ifndef CSDF_OUTPUT_BUFFER_THROUGHPUT_BUFFY_H_INCLUDED
#define CSDF_OUTPUT_BUFFER_THROUGHPUT_BUFFY_H_INCLUDED

#include "../../csdf.h"

/**
 * outputCSDFGasBuffyModel ()
 * Output the CSDFG as a buffy model.
 */
void outputCSDFGasBuffyModel(TimedCSDFgraph *g, ostream &out);

#endif
