/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   binding.cc
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
 * $Id: binding.cc,v 1.5 2008/03/06 10:49:45 sander Exp $
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

#include "binding.h"
#include "../../transform/model/autoconc.h"
#include "../../transform/model/buffersize.h"
#include "../../output/xml/xml.h"

/**
 * analyzeThroughputApplication ()
 * The function returns the throughput of the application graph
 * considering the absense of auto-concurrency, the buffer constraints
 * and the mapping of actors to the fastest processor.
 */
double Binding::analyzeThroughputApplication()
{
    SDFstateSpaceThroughputAnalysis thrAnalysisAlgo;
    TimedSDFgraph *g, *gr;
    double thrGraph;

    // Model absense of auto-concurrency in SDFG
    g = (TimedSDFgraph*)modelAutoConcurrencyInSDFgraph(appGraph, 1);
    
    // Set buffer size of each channel to max(mem, src+dst)
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)*iter;
        TimedSDFchannel::BufferSize sz = c->getBufferSize();
        
        if (sz.mem < sz.src + sz.dst)
        {
            sz.sz = sz.src+sz.dst;
            c->setBufferSize(sz);
        }    
    }
    
    // Model buffer constraints in SDFG
    gr = modelBufferSizeInSDFgraph(g);

    // Select fastest processor for each actor as the default processor
    for (SDFactorsIter iter = gr->actorsBegin(); 
            iter != gr->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;
        TimedSDFactor::Processor *proc = NULL;
        
        for (TimedSDFactor::ProcessorsIter iterP = a->processorsBegin();
                iterP != a->processorsEnd(); iterP++)
        {
            TimedSDFactor::Processor *p = *iterP;
            
            if (proc == NULL || proc->execTime > p->execTime)
                proc = p;
        }
        
        if (proc != NULL)
            a->setDefaultProcessor(proc->type);
    }

    // Compute throughput of the SDFG
    thrGraph = thrAnalysisAlgo.analyze(gr);

    // Cleanup
    delete g;
    delete gr;
    
    return thrGraph;
}

/**
 * analyzeThroughput ()
 * The function returns the throughput of the application graph
 * mapped onto the platform graph.
 */
double Binding::analyzeThroughput(vector<double> &tileUtilization)
{
    SDFstateSpaceBindingAwareThroughputAnalysis thrAnalysisAlgo;
    BindingAwareSDFG *bindingAwareSDFG;
    double thrGraph;

    // Create binding-aware SDFG
    bindingAwareSDFG = new BindingAwareSDFG(appGraph, archGraph, flowType);

    // Compute throughput of mapped SDF graph
    thrGraph = thrAnalysisAlgo.analyze(bindingAwareSDFG, tileUtilization);

    // Cleanup
    delete bindingAwareSDFG;

    return thrGraph;
}

/**
 * isThroughputConstraintSatisfied ()
 * Check wether or not the throughput constraint is satisfied.
 */
bool Binding::isThroughputConstraintSatisfied()
{
    double thrGraph, thrConstraint;
    vector<double> tileUtilization;
    
    // Compute throughput of mapped SDF graph
    thrGraph = analyzeThroughput(tileUtilization);
    thrConstraint = appGraph->getThroughputConstraint().value();

    #ifdef VERBOSE
    cerr << "Throughput: " << thrGraph << endl;
    cerr << "Constraint: " << thrConstraint << endl;
    #endif

    // Is throughput constraint met?
    if (thrConstraint > thrGraph)
    {
        #ifdef VERBOSE
        cerr << "Failed throughput constraint." << endl;
        #endif
        
        return false;
    }

    #ifdef VERBOSE
    cerr << "Throughput constraint satisfied." << endl;
    #endif
    
    return true;
}

/**
 * minimizeStaticOrderSchedules ()
 * The function minimizes the length of all static-order schedules.
 */
void Binding::minimizeStaticOrderSchedules(PlatformGraph *archGraph)
{
    // Iterate over the tiles
    for (TilesIter iter = archGraph->tilesBegin();
            iter != archGraph->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        
        // Processor inside the tile
        if (p != NULL)
        {
            StaticOrderSchedule &s = p->getSchedule();
            
            // Minimize the schedule
            s.minimize();

            logMsg("Reduced schedule on '" + t->getName() + "' to "
                    + CString(s.size()) + " states.");
        }
    }
}

