/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffer_ning_gao.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 31, 2008
 *
 *  Function        :   State-space based buffer size analysis for Ning and 
 *                      Gao's buffer sizing problem
 *
 *  History         :
 *      31-03-08    :   Initial version.
 *
 * $Id: buffer_ning_gao.h,v 1.2 2008/09/18 07:38:21 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_BUFFER_NING_GAO_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_BUFFER_NING_GAO_H_INCLUDED

#include "storage_distribution.h"
#include "../../base/timed/graph.h"

/**
 * Throughput / storage-space trade-off exploration for Ning and Gao's buffer 
 * sizing problem. Analyze the trade-offs between storage distributions and 
 * throughput.
 */
class SDFstateSpaceBufferAnalysisNingGao
{
public:
    // Constructor
    SDFstateSpaceBufferAnalysisNingGao() {};
    
    // Destructor
    ~SDFstateSpaceBufferAnalysisNingGao() {};
    
    // Analyze throughput/storage-space trade-off space
    StorageDistribution *analyze(TimedSDFgraph *gr, vector<SDFtime> &startTime);
    
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
        TDtime execSDFgraph(const TBufSize *sp, bool *dep, 
                vector<SDFtime> &startTime);
        
    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);
        
        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Dependencies
        void dfsVisitDependencies(uint a, int *color, int *pi,
                bool **abstractDepGraph, bool *dep);
        void findStorageDependencies(bool **abstractDepGraph, bool *dep);
        void findCausalDependencies(SDFactor *a, bool **abstractDepGraph);
        void analyzePeriodicPhase(const TBufSize *sp, bool *dep);
        void analyzeDeadlock(const TBufSize *sp, bool *dep);

        // Compute throughput from transition system
        TDtime computeThroughput(const StatesIter cycleIter);

        // State transitions
        bool releaseStorageSpaceSharedOutputBuffer(SDFchannel *c);
        bool actorReadyToFire(SDFactor *a);
        void startActorFiring(TimedSDFactor *a);
        bool actorReadyToEnd(SDFactor *a);
        void endActorFiring(SDFactor *a);
        SDFtime clockStep();

        // Output actor
        void initOutputActor();

        // SDF graph and output actor
        TimedSDFgraph *g;
        SDFactor *outputActor;
        TCnt outputActorRepCnt;
        
        // Current and previous state
        State currentState; 
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };
    
    // Bounds on the search space
    void initBoundsSearchSpace(TimedSDFgraph *g);
    void initMinimalChannelSzStep(TimedSDFgraph *g);
    void initMinimalChannelSz(TimedSDFgraph *g);
    void initLbDistributionSz(TimedSDFgraph *g);
    void initMaxThroughput(TimedSDFgraph *g);
    
    // Storage distributions
    StorageDistribution *newStorageDistribution();
    void deleteStorageDistribution(StorageDistribution *d);
    void execStorageDistribution(StorageDistribution *d, 
            vector<SDFtime> &startTime);
    void minimizeStorageDistributionsSet(StorageDistributionSet *ds);
    bool addStorageDistributionToChecklist(StorageDistribution *d);
    void exploreStorageDistribution(StorageDistributionSet *ds,
            StorageDistribution *d);
    void exploreStorageDistributionSet(StorageDistributionSet *ds);
    void findMinimalStorageDistributions(const double thrBound);
    
    // SDF graph
    TimedSDFgraph *g;
    
    // Transition system
    TransitionSystem *transitionSystem;
    
    // Storage distributions
    StorageDistributionSet *minStorageDistributions;
    StorageDistributionSet *lastExploredStorageDistributionSet;
    
    // Bounds on the search space
    TBufSize *minSz;
    TBufSize *minSzStep;
    TBufSize lbDistributionSz;
    TDtime maxThroughput;
};

#endif

