/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   list_scheduler.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 24, 2006
 *
 *  Function        :   Construct a list schedule for each processor in the
 *                      platform graph.
 *
 *  History         :
 *      24-04-06    :   Initial version.
 *
 * $Id: list_scheduler.h,v 1.3 2008/03/06 10:49:45 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_SCHEDULING_LIST_SCHEDULER_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_SCHEDULING_LIST_SCHEDULER_H_INCLUDED

#include "../../analysis/analysis.h"

/**
 * List scheduler
 * Constructs a set of static-order schedules for the processors in the
 * architecture platform. Each schedule orders the firings of the actors
 * running on the processor. A list scheduler is used to construct the
 * static-order schedules.
 */
class SDFstateSpaceListScheduler
{
public:
    // Constructor
    SDFstateSpaceListScheduler() {};
    
    // Destructor
    ~SDFstateSpaceListScheduler() {};
    
    // Schedule the graph
    void schedule(BindingAwareSDFG *bg);

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
            initOutputActor();
            initStaticOrderSchedules();
        };
        
        // Destructor
        ~TransitionSystem() {};

        // Execute the SDFG
        TDtime execSDFgraph();
        
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

        // Initialize static-order schedules
        void initStaticOrderSchedules();
        
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

        // State information for schedulers        
        vector< SDFactors > actorReadyList;
        vector < bool > procIdle;
    };
};

#endif
