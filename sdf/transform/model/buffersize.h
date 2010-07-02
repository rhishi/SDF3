/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffersize.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 19, 2005
 *
 *  Function        :   Model buffer sizes in the graph with explicit channels.
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: buffersize.h,v 1.2 2008/09/18 07:38:21 sander Exp $
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

#ifndef SDF_TRANSFORM_MODEL_BUFFERSIZE_H_INCLUDED
#define SDF_TRANSFORM_MODEL_BUFFERSIZE_H_INCLUDED

#include "../../base/timed/graph.h"

/**
 * modelBufferSizeInSDFgraph ()
 * Create a new timed SDF graph in which channel sizes are modelled
 * through explicit channels (all buffer sizes are set to unbounded).
 */
TimedSDFgraph *modelBufferSizeInSDFgraph(const TimedSDFgraph *g);

/**
 * createCapacityConstrainedModel ()
 * Create a new timed HSDF graph in which every actor 'a' of the original graph 
 * 'g' belongs to a cycle consisting of the actor 'a' and a new actor 'aMCM'.
 * This cycle contains one initial token and its cycle mean is equal to the MCM
 * of the graph g when g is executed without auto-concurrency.
 */
TimedSDFgraph *createCapacityConstrainedModel(TimedSDFgraph *g);

/**
 * modelCapacityConstrainedBuffer ()
 * Create a new timed HSDF graph in which channel sizes are modelled
 * through explicit channels (all buffer sizes are set to unbounded).
 * The function 'getStorageSpaceChannel()' on each channel gives a
 * pointer to the channel in input graph g for which the storage space
 * is determined.
 */
TimedSDFgraph *modelCapacityConstrainedBuffer(TimedSDFgraph *g, const uint mcm);

#endif
