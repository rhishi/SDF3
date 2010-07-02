/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffer.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 5, 2006
 *
 *  Function        :   State-space based buffer size analysis
 *
 *  History         :
 *      05-04-06    :   Initial version.
 *      27-10-06    :   BFS based version of the trade-off space exploration
 *                      algorithm.
 *
 * $Id: buffer.h,v 1.2 2008/04/09 15:02:00 sander Exp $
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

#ifndef CSDF_ANALYSIS_STATESPACE_BUFFER_H_INCLUDED
#define CSDF_ANALYSIS_STATESPACE_BUFFER_H_INCLUDED

#include "sdf/sdf.h"
#include "../../base/timed/graph.h"

/**
 * Throughput / storage-space trade-off exploration
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency). The search ends as soon as the throughput bound (thrBound)
 * is reached. To find the complete pareto-space, the throughput bound should
 * be set to DOUBLE_MAX.
 */
class CSDFstateSpaceBufferAnalysis
{
public:
    // Constructor
    CSDFstateSpaceBufferAnalysis() {};
    
    // Destructor
    ~CSDFstateSpaceBufferAnalysis() {};
    
    // Analyze throughput/storage-space trade-off space
    StorageDistributionSet *analyze(TimedCSDFgraph *gr,
            const double thrBound = DBL_MAX);

    // Analyze throughput/storage-space trade-off space step-by-step
    void initSearch(TimedCSDFgraph *gr);
    StorageDistributionSet *findNextStorageDistributionSet();
    
    // Approximation technique
    StorageDistributionSet *approximate(TimedCSDFgraph *gr, const double thr,
            const double maxPercentageOverEstimation);

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
                sp.resize(nrChannels);
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
            vector< TBufSize > sp;
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

        // Execute the SDFG
        TDtime execCSDFgraph(const TBufSize *sp, bool *dep);
        
    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);
        
        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Dependencies
        void dfsVisitDependencies(uint a, int *color, int *pi,
                bool **abstractDepGraph, bool *dep);
        void findStorageDependencies(bool **abstractDepGraph, bool *dep);
        void findCausalDependencies(CSDFactor *a, bool **abstractDepGraph);
        void analyzePeriodicPhase(const TBufSize *sp, bool *dep);
        void analyzeDeadlock(const TBufSize *sp, bool *dep);

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

        // SDF graph and output actor
        TimedCSDFgraph *g;
        CSDFactor *outputActor;
        TCnt outputActorRepCnt;
        
        // Current and previous state
        State currentState; 
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };
    
    // Bounds on the search space
    void initBoundsSearchSpace(TimedCSDFgraph *g);
    void initMinimalChannelSzStep(TimedCSDFgraph *g);
    void initMinimalChannelSz(TimedCSDFgraph *g);
    void initLbDistributionSz(TimedCSDFgraph *g);
    void initMaxThroughput(TimedCSDFgraph *g);
    
    // Storage distributions
    StorageDistribution *newStorageDistribution();
    void deleteStorageDistribution(StorageDistribution *d);
    void execStorageDistribution(StorageDistribution *d);
    void minimizeStorageDistributionsSet(StorageDistributionSet *ds);
    bool addStorageDistributionToChecklist(StorageDistribution *d);
    void exploreStorageDistribution(StorageDistributionSet *ds,
            StorageDistribution *d);
    void exploreStorageDistributionSet(StorageDistributionSet *ds);
    void findMinimalStorageDistributions(const double thrBound);
    
    // CSDF graph
    TimedCSDFgraph *g;
    
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
    
    // Approximation technique
    TBufSize *lastChStep;
    uint quantizationFactor;
};

#endif

