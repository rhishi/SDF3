/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   bounded_buffer.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 24, 2006
 *
 *  Function        :   Compute throughput/buffer trade-off for an SDFG which
 *                      is bound to an multi-processor architecture.
 *
 *  History         :
 *      24-04-06    :   Initial version.
 *
 * $Id: bounded_buffer.h,v 1.1 2008/03/06 10:49:42 sander Exp $
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


#ifndef SDF_ANALYSIS_STATESPACE_BOUNDED_BUFFER_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_BOUNDED_BUFFER_H_INCLUDED

#include "storage_distribution.h"
#include "../../resource_allocation/binding_aware_sdfg/binding_aware_sdfg.h"

/**
 * Throughput / storage-space trade-off exploration
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency) under the given binding constraints of the application
 * to the architecture graph. The vector 'bufferChannels' indicates for each
 * channel wether initial tokens can be added to it. This is only true for
 * channels modeling buffer space. When the 'useBoundsOnBufferChannels' flag
 * is true, the number of initial tokens (i.e. storage space) is not enlarged
 * above the current number of initial tokens on the edge (i.e. current storage
 * space). This storage space allocation is then a constraint on the maximum
 * amount of storage space that can be allocated to the channel. The exploration
 * ends as soon as the throughput reaches the throughput bound (thrBound) or all
 * minimal storage distributions are found (default).
 */
class SDFstateSpaceBindingAwareBufferAnalysis
{
public:
    // Constructor
    SDFstateSpaceBindingAwareBufferAnalysis() {};
    
    // Destructor
    ~SDFstateSpaceBindingAwareBufferAnalysis() {};
    
    // Analyze throughput/storage-space trade-off space
    StorageDistributionSet *analyze(BindingAwareSDFG *bg, bool *bufferChannels,
            bool useBoundsOnBufferChannels, const double thrBound);

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
            State(const uint nrActors = 0, const uint nrChannels = 0, 
                    const uint nrTiles = 0) { 
                init(nrActors, nrChannels, nrTiles);
            };

            // Destructor
            ~State(){};

            // Initialize the state    
            void init(const uint nrActors, const uint nrChannels,
                    const uint nrTiles)
            {
                actClk.resize(nrActors);
                ch.resize(nrChannels);
                schedulePos.resize(nrTiles);
                tdmaPos.resize(nrTiles);
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
            vector< uint > schedulePos;
            vector< SDFtime > tdmaPos;
        };

        // Sequence of states in transition system        
        typedef list<State>      States;
        typedef States::iterator StatesIter;

        // Constructor
        TransitionSystem(BindingAwareSDFG *bg) {
            bindingAwareSDFG = bg;
            checkBindingAwareSDFG();
            initOutputActor();
        };
        
        // Destructor
        ~TransitionSystem() {};

        // Execute the SDFG
        TDtime execSDFgraph(const TBufSize *sp, bool *dep,
                bool *bufferChannels);
        
    private:
        // Store state
        bool storeState(State &s, StatesIter &pos);
        
        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Dependencies
        void dfsVisitDependencies(uint a, int *color, int *pi,
                bool **abstractDepGraph, bool *dep);
        void findStorageDependencies(bool **abstractDepGraph, bool *dep, 
                bool *bufferChannels);
        void findCausalDependencies(SDFactor *a, bool **abstractDepGraph);
        void analyzePeriodicPhase(bool *dep, bool *bufferChannels);
        void analyzeDeadlock(bool *dep, bool *bufferChannels);

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

        // Sanity checks on the binding-aware SDFG
        void checkBindingAwareSDFG();
        
        // Binding-aware SDFG
        BindingAwareSDFG *bindingAwareSDFG;

        // SDF graph and output actor
        SDFactor *outputActor;
        TCnt outputActorRepCnt;
        
        // Current and previous state
        State currentState; 
        State previousState;

        // List of visited states that are stored
        States storedStates;
    };
    
    // Bounds on the search space
    void initBoundsSearchSpace(TimedSDFgraph *g, bool *bufferChannels);
    void initMinimalChannelSzStep(TimedSDFgraph *g);
    void initMinimalChannelSz(TimedSDFgraph *g, bool *bufferChannels);
    void initLbDistributionSz(TimedSDFgraph *g);
    void initMaxThroughput(TimedSDFgraph *g);
    
    // Storage distributions
    StorageDistribution *newStorageDistribution();
    void deleteStorageDistribution(StorageDistribution *d);
    void execStorageDistribution(StorageDistribution *d, bool *bufferChannels);
    void minimizeStorageDistributionsSet(StorageDistributionSet *ds);
    bool addStorageDistributionToChecklist(StorageDistribution *d);
    void exploreStorageDistribution(StorageDistributionSet *ds,
            StorageDistribution *d, bool *bufferChannels,
            bool useBoundsOnBufferChannels);
    void exploreStorageDistributionSet(StorageDistributionSet *ds,
            bool *bufferChannels, bool useBoundsOnBufferChannels);
    void findMinimalStorageDistributions(const double thrBound,
            bool *bufferChannels, bool useBoundsOnBufferChannels);
    
    // Binding-aware SDFG
    BindingAwareSDFG *bindingAwareSDFG;
    
    // Transition system
    TransitionSystem *transitionSystem;
    
    // Storage distributions
    StorageDistributionSet *minStorageDistributions;
    
    // Bounds on the search space
    TBufSize *minSz;
    TBufSize *minSzStep;
    TBufSize lbDistributionSz;
    TDtime maxThroughput;
};

#endif
