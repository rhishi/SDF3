/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   selftimed_minimal.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 12, 2008
 *
 *  Function        :   State-space based latency analysis
 *
 *  History         :
 *      12-02-08    :   Initial version.
 *
 * $Id: selftimed_minimal.h,v 1.1 2008/03/06 10:49:44 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_LATENCY_SELFTIMED_MINIMAL_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_LATENCY_SELFTIMED_MINIMAL_H_INCLUDED

#include "../../base/timed/graph.h"

/**
 * Selftimed latency analysis
 * Compute the minimal latency an SDF graph under maximal throughput.
 */
class SDFstateSpaceSelfTimedMinimalLatencyAnalysis
{
public:
    // Constructor
    SDFstateSpaceSelfTimedMinimalLatencyAnalysis() {};
    
    // Destructor
    ~SDFstateSpaceSelfTimedMinimalLatencyAnalysis() {};
    
    // Analyze latency of the graph
    void analyze(TimedSDFgraph *g, SDFactor *srcActor, SDFactor *dstActor,
                    TDtime &latency, TDtime &throughput);

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

        // Timing constraint on a firing
        typedef struct _TimingConstraintFiring
        {
            CId id;
            SDFtime timeTillNextFiring;
            uint nrFiringsEnabled;
            TransitionSystem::State lastStoredState;
            bool visit;
            struct _TimingConstraintFiring *next;
        } TimingConstraintFiring;

        // Constructor
        TransitionSystem(TimedSDFgraph *gr, SDFactor *outActor) {
            g = gr;
            initOutputActor(outActor);   
        };
        
        // Destructor
        ~TransitionSystem() {};

        // Execute the SDFG
        TimingConstraintFiring *execSDFgraph(SDFactor *dstActor);
        
        // Execute the SDFG following the timing constraints
        TDtime execConstrainedSDFgraph(SDFactor *srcActor,
                TimingConstraintFiring *timeSrcFire, double latency,
                SDFactor *dstActor, vector<SDFtime> &srcTimeFire,
                vector<SDFtime> &dstTimeFire, uint distance);
        
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
        SDFtime clockStep(const uint maxStep = UINT_MAX);

        // Output actor
        void initOutputActor(SDFactor *outActor);

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
