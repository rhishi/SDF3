/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 11, 2006
 *
 *  Function        :   Base class for all binding algorithms
 *
 *  History         :
 *      11-04-06    :   Initial version.
 *
 * $Id: binding.h,v 1.2 2008/03/06 10:49:45 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_TILE_ALLOCATION_BINDING_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_TILE_ALLOCATION_BINDING_H_INCLUDED

#include "../mpsoc_arch/graph.h"
#include "../../analysis/analysis.h"

#define VERBOSE true

/**
 * Binding ()
 * Base class for all binding algorithm.
 */
class Binding
{
public:
    // Constructor
    Binding(SDFflowType type) { 
        archGraph = NULL; 
        appGraph = NULL;
        flowType = type; 
    };
    
    // Destructor
    virtual ~Binding() {};

    // Application graph
    TimedSDFgraph *getAppGraph() { return appGraph; };
    virtual void setAppGraph(TimedSDFgraph *g) =0;
    
    // Architecture graph
    PlatformGraph *getArchGraph() { return archGraph; };
    void setArchGraph(PlatformGraph *g) { archGraph = g; };
    
    // Flow
    SDFflowType getFlowType() const { return flowType; };
    
    // Binding algorithm
    virtual bool bind() =0;
    virtual bool bindSDFGtoTiles() =0;
    virtual bool constructStaticOrderScheduleTiles() =0;
    virtual bool allocateTDMAtimeSlices() =0;
    virtual bool optimizeStorageSpaceAllocations() =0;

    // Resource management
    virtual void releaseResources() =0;

protected:
   // Throughput
   double analyzeThroughputApplication();
   double analyzeThroughput(vector<double> &tileUtilization);
   bool isThroughputConstraintSatisfied();

    // Static order schedule
    void minimizeStaticOrderSchedules(PlatformGraph *archGraph);

protected:
   // Architecture graph
   PlatformGraph *archGraph;
   
   // Application graph
   TimedSDFgraph *appGraph;
   
   // Flow
   SDFflowType flowType;
};

#endif

