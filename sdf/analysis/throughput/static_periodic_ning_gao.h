/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_periodic_ning_gao.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 31, 2008
 *
 *  Function        :   State-space based throughput analysis for Ning and 
 *                      Gao's buffer sizing problem
 *
 *  History         :
 *      31-03-08    :   Initial version.
 *
 * $Id: static_periodic_ning_gao.h,v 1.1 2008/09/18 07:35:13 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_THROUGHPUT_STATIC_PERIODIC_NING_GAO_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_THROUGHPUT_STATIC_PERIODIC_NING_GAO_H_INCLUDED

#include "../buffersizing/storage_distribution.h"
#include "../../base/timed/graph.h"

/**
 * Throughput / storage-space trade-off exploration for Ning and Gao's buffer 
 * sizing problem. Analyze the trade-offs between storage distributions and 
 * throughput.
 */
class SDFstateSpaceThroughputAnalysisNingGao
{
public:
    // Constructor
    SDFstateSpaceThroughputAnalysisNingGao() {};
    
    // Destructor
    ~SDFstateSpaceThroughputAnalysisNingGao() {};
    
    // Analyze throughput
    TDtime analyze(TimedSDFgraph *gr, StorageDistribution *d, 
                    SDFtime period, vector<SDFtime> &startTime);
    
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
                sp.resize(nrActors);
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
            vector< TBufSize > sp;
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
        TDtime execSDFgraph(const TBufSize *sp, SDFtime &period, 
                vector<SDFtime> &startTime);
        
    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);
        
        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter cycleIter);

        // State transitions
        bool releaseStorageSpaceSharedOutputBuffer(SDFchannel *c);
        bool actorReadyToFire(SDFactor *a);
        void startActorFiring(TimedSDFactor *a);
        bool actorReadyToEnd(SDFactor *a);
        void endActorFiring(SDFactor *a);
        SDFtime clockStep(vector<SDFtime> &startTime, vector<uint> &fireCnt, 
                SDFtime &period, TTime &globalTime);

        // Output actor
        void initOutputActor();

        // SDF graph and output actor
        TimedSDFgraph *g;
        SDFactor *outputActor;
        TCnt outputActorRepCnt;
        
        // Current state
        State currentState; 

        // List of visited states that are stored
        States storedStates;
    };
    
    // SDF graph
    TimedSDFgraph *g;
    
    // Transition system
    TransitionSystem *transitionSystem;
};

#endif

