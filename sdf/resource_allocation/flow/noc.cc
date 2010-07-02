/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   noc.cc
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
 * $Id: noc.cc,v 1.1.1.1 2007/10/02 10:59:48 sander Exp $
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
 * extractCommunicationConstraints ()
 * Extract communication scheduling problem from the bound and scheduled
 * application.
 */
void SDF3Flow::extractCommunicationConstraints()
{
    // Output current state of the flow
    logInfo("Communication constraint extraction.");

    if (!nocMapping->extractCommunicationConstraints())
        setNextStateOfFlow(FlowFailed);
    else
        setNextStateOfFlow(FlowScheduleCommunication);
}

/**
 * scheduleCommunication ()
 * The function tries to find a valid schedule function for the communication
 * scheduling problem.
 */
void SDF3Flow::scheduleCommunication()
{
    // Output current state of the flow
    logInfo("Communication scheduling.");

    if (!nocMapping->scheduleCommunication())
        setNextStateOfFlow(FlowSelectStorageDist);
    else
        setNextStateOfFlow(FlowUpdateBandwidthAllocations);
}

/**
 * updateBandwidthAllocations ()
 * The function computes the minimal amount of bandwidth needed on the NIs and
 * it updates these allocations accordingly.
 */
void SDF3Flow::updateBandwidthAllocations()
{
    // Output current state of the flow
    logInfo("Update bandwidth allocations.");

    if (!nocMapping->updateBandwidthAllocations())
        setNextStateOfFlow(FlowFailed);
    else
        setNextStateOfFlow(FlowCompleted);
}
