/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   selftimed_throughput.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   October 10, 2007
 *
 *  Function        :   State-space based throughput analysis
 *
 *  History         :
 *      10-10-07    :   Initial version.
 *
 * $Id: selftimed_throughput.h,v 1.1 2008/03/22 14:24:21 sander Exp $
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

#ifndef CSDF_ANALYSIS_STATESPACE_SELFTIMED_THROUGHPUT_H_INCLUDED
#define CSDF_ANALYSIS_STATESPACE_SELFTIMED_THROUGHPUT_H_INCLUDED

#include "../../base/timed/graph.h"

/**
 * Throughput analysis
 * Compute the throughput of an CSDF graph for unconstrained buffer sizes and
 * using auto-concurrency using a state-space traversal.
 */
class CSDFstateSpaceThroughputAnalysis
{
public:
    // Constructor
    CSDFstateSpaceThroughputAnalysis() {};
    
    // Destructor
    ~CSDFstateSpaceThroughputAnalysis() {};
    
    // Analyze throughput of the graph
    TDtime analyze(TimedCSDFgraph *g);

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
                actSeqPos.resize(nrActors);
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
            vector< list<CSDFtime> > actClk;
            vector< uint > actSeqPos;
            vector< TBufSize > ch;
            unsigned long glbClk;
        };

        // Sequence of states in transition system        
        typedef list<State>      States;
        typedef States::iterator StatesIter;

        // Constructor
        TransitionSystem(TimedCSDFgraph *gr) {
            g = gr;
            initOutputActor();   
        };
        
        // Destructor
        ~TransitionSystem() {};

        // Execute the CSDFG
        TDtime execCSDFgraph();
        
    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);
        
        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter cycleIter);

        // State transitions
        bool actorReadyToFire(CSDFactor *a);
        void startActorFiring(TimedCSDFactor *a);
        bool actorReadyToEnd(CSDFactor *a);
        void endActorFiring(CSDFactor *a);
        CSDFtime clockStep();

        // Output actor
        void initOutputActor();

        // CSDF graph and output actor
        TimedCSDFgraph *g;
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

