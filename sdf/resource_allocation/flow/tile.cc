/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tile.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 7, 2007
 *
 *  Function        :   SDFG mapping to MP-SoC
 *
 *  History         :
 *      07-02-07    :   Initial version.
 *
 * $Id: tile.cc,v 1.2 2008/03/06 10:49:44 sander Exp $
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

#include "flow.h"

/**
 * bindSDFGtoTiles ()
 * Bind actor and channels of the SDFG to the tile resources.
 */
void SDF3Flow::bindSDFGtoTiles()
{
    // Output current state of the flow
    logInfo("Bind SDFG to tiles.");

    // Release existing binding (if needed)
    tileMapping->releaseResources();

    if (!tileMapping->bindSDFGtoTiles())
        setNextStateOfFlow(FlowSelectStorageDist);
    else
        setNextStateOfFlow(FlowStaticOrderScheduleTiles);
}

/**
 * constructStaticOrderScheduleTiles ()
 * Constrauct static-order schedule for every tile to which actors
 * are bound.
 */
void SDF3Flow::constructStaticOrderScheduleTiles()
{
    // Output current state of the flow
    logInfo("Construct static-order schedules per tile.");

    if (!tileMapping->constructStaticOrderScheduleTiles())
        setNextStateOfFlow(FlowFailed);
    else
        setNextStateOfFlow(FlowAllocateTDMAtimeSlices);
}

/**
 * allocateTDMAtimeSlices ()
 * Allocate TDMA time slices on the processors executing actors from the
 * application.
 */
void SDF3Flow::allocateTDMAtimeSlices()
{
    // Output current state of the flow
    logInfo("Allocate TDMA time-slices.");

    if (!tileMapping->allocateTDMAtimeSlices())
        setNextStateOfFlow(FlowSelectStorageDist);
    else
        setNextStateOfFlow(FlowOptimizeStorageSpaceAllocations);
}

/**
 * optimizeStorageSpaceAllocations ()
 * Optimize the storage space allocations.
 */
void SDF3Flow::optimizeStorageSpaceAllocations()
{
    // Output current state of the flow
    logInfo("Optimize storage space allocations.");

    if (!tileMapping->optimizeStorageSpaceAllocations())
        setNextStateOfFlow(FlowFailed);
    else if (getFlowType() == SDFflowTypeMPFlow)
        setNextStateOfFlow(FlowCompleted);
    else
        setNextStateOfFlow(FlowExtractCommunicationConstraints);    
}

