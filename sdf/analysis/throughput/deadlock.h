/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   deadlock.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 13, 2007
 *
 *  Function        :   State-space based deadlock analysis
 *
 *  History         :
 *      13-11-07    :   Initial version.
 *
 * $Id: deadlock.h,v 1.1 2008/03/06 10:49:44 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_DEADLOCK_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_DEADLOCK_H_INCLUDED

#include "../../base/timed/graph.h"

/**
 * Deadlock analysis
 * Analyze whether an SDFG is deadlock free or not.
 */
class SDFstateSpaceDeadlockAnalysis
{
public:
    // Constructor
    SDFstateSpaceDeadlockAnalysis() {};
    
    // Destructor
    ~SDFstateSpaceDeadlockAnalysis() {};
    
    // Analyze absense of deadlock of the graph
    bool isDeadlockFree(SDFgraph *g);

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
                ch.resize(nrChannels);
            };

            // Compare states
            bool operator==(const State &s);

            // Clear state
            void clear();

            // Output state
            void print(ostream &out);

            // State information
            vector< TBufSize > ch;
        };

        // Sequence of states in transition system        
        typedef list<State>      States;
        typedef States::iterator StatesIter;

        // Constructor
        TransitionSystem(SDFgraph *gr) {
            g = gr;
        };
        
        // Destructor
        ~TransitionSystem() {};

        // Execute the SDFG
        bool execSDFgraph();
        
    private:
        // State transitions
        bool actorReadyToFire(SDFactor *a);
        void fireActor(SDFactor *a);

        // SDF graph and output actor
        SDFgraph *g;
        
        // Current state
        State initialState;
        State currentState; 
    };
};

#endif
