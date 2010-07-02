/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   minimal.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 13, 2008
 *
 *  Function        :   State-space based latency analysis
 *
 *  History         :
 *      13-02-08    :   Initial version.
 *
 * $Id: minimal.h,v 1.1 2008/03/06 10:49:43 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_LATENCY_MINIMAL_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_LATENCY_MINIMAL_H_INCLUDED

#include "../../base/timed/graph.h"

/**
 * Minimal latency analysis
 * Compute the minimal latency an SDF graph 
 */
class SDFstateSpaceMinimalLatencyAnalysis
{
public:
    // Constructor
    SDFstateSpaceMinimalLatencyAnalysis() {};
    
    // Destructor
    ~SDFstateSpaceMinimalLatencyAnalysis() {};
    
    // Analyze latency of the graph (infinitly many compute resources)
    void analyze(TimedSDFgraph *g, SDFactor *srcActor, SDFactor *dstActor,
                    TDtime &latency, TDtime &throughput);

    // Analyze latency of the graph (single compute resources)
    void analyzeSingleProc(TimedSDFgraph *g, SDFactor *srcActor,
                    SDFactor *dstActor, TDtime &latency);

private:

    /***************************************************************************
     * Transition system
     **************************************************************************/
    class TransitionSystem
    {
    public:
    
        /***********************************************************************
         * State
         **********************************************************************/
        class State
        {
        public:
            // Constructor
            State(const uint nrActors = 0, const uint nrChannels = 0) { 
                init(nrActors, nrChannels);
            };

            // Destructor
            ~State(){};

            // Initialize the state    
            void init(const uint nrActors, const uint nrChannels)
            {
                actClk.resize(nrActors);
                ch.resize(nrChannels);
            };

            // Compare states
            bool operator==(const State &s);

            // Clear state
            void clear();

            // Output state
            void print(ostream &out);

            // State information
            vector< list<SDFtime> > actClk;
            vector< TBufSize > ch;
            unsigned long glbClk;
        };

        // Sequence of states in transition system        
        typedef list<State>      States;
        typedef States::iterator StatesIter;

        // Constructor
        TransitionSystem(TimedSDFgraph *gr) {
            g = gr;
            initOutputActor();   
        };
        
        // Destructor
        ~TransitionSystem() {};

        // Execute the SDFG
        TDtime execSDFgraph(bool startFromInitialState = true);
        
        // Execute the SDFG following the demand list
        SDFtime execSDFgraphUsingDemandList(vector<uint> &demandList);
        
        // Demand list
        void computeDemandList(const SDFchannel *ch, vector<uint> &demandList);

        // State
        State &getCurrentState() { return currentState; };
        void setCurrentState(State &s) { currentState = s; };
        State &getPreviousState() { return previousState; };
        void setPreviousState(State &s) { previousState = s; };

    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);
        
        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter cycleIter);

        // State transitions
        bool actorReadyToFire(SDFactor *a);
        void startActorFiring(TimedSDFactor *a);
        bool actorReadyToEnd(SDFactor *a);
        void endActorFiring(SDFactor *a);
        SDFtime clockStep();

        // Output actor
        void initOutputActor();

        // SDF graph and output actor
        TimedSDFgraph *g;
        CId outputActor;
        TCnt outputActorRepCnt;
        
        // Current and previous state
        State currentState; 
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };
};

#endif
