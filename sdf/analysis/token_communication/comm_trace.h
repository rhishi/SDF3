/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   comm_trace.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 24, 2006
 *
 *  Function        :   Extract timing constraints for interconnect
 *                      communication from the state-space.
 *
 *  History         :
 *      24-04-06    :   Initial version.
 *
 * $Id: comm_trace.h,v 1.1 2008/03/06 10:49:44 sander Exp $
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

#ifndef SDF_ANALYSIS_STATESPACE_COMM_TRACE_H_INCLUDED
#define SDF_ANALYSIS_STATESPACE_COMM_TRACE_H_INCLUDED

#include "../../resource_allocation/binding_aware_sdfg/binding_aware_sdfg.h"

/**
 * Trace interconnect communication
 * Generates a trace of all communication over the interconnect
 * of a mapped and scheduled SDFG on a NoC-based platform. The relation 
 * between the communication in the transient and periodic part of the
 * schedule is also included.
 */
class SDFstateSpaceTraceInterconnectCommunication
{
public:
    // Constructor
    SDFstateSpaceTraceInterconnectCommunication() {};
    
    // Destructor
    ~SDFstateSpaceTraceInterconnectCommunication() {};
    
    // Trace timing-constraints of communication
    CNode *trace(BindingAwareSDFG *bg, PlatformGraph *platformGraph, 
                    uint slotTableSize);

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
        TransitionSystem(BindingAwareSDFG *bg, PlatformGraph *pg) {
            bindingAwareSDFG = bg;
            platformGraph = pg;
            checkBindingAwareSDFG();
            initOutputActor();
            initTracing();
        };
        
        // Destructor
        ~TransitionSystem();

        // Trace the communication
        CNode *traceCommunication(uint slotTableSize);

    private:
        // Execute the SDFG
        TTime execSDFgraph();

        // Store state
        bool storeState(State &s, StatesIter &pos);
        
        // Clear list of stored states
        void clearStoredStates() { storedStates.clear(); };

        // Tracing
        void initTracing();
        TTime computeLengthPeriodicPhase(const StatesIter cycleIter);
        void traceConsumptionToken(const CId &ch, const uint &rate,
                const SDFtime &delay);
        void traceProductionToken(const CId &ch, const uint &rate);
        void traceAdvanceClocks(const SDFtime step);
        bool isTracingCompleted();
        void traceMarkTokensInPeriodicPhase();
        
        // State transitions
        bool actorReadyToFire(SDFactor *a);
        void startActorFiring(TimedSDFactor *a);
        bool actorReadyToEnd(SDFactor *a);
        void endActorFiring(SDFactor *a);
        SDFtime clockStep();
        TTime traceMessagesTransient(TTime shiftStartTransPhase, 
            TTime shiftEndTransPhase, CNode *msgTransNode);
        TTime traceMessagesPeriodic(uint duplicationPeriodicPhase, 
            TTime lengthPeriodicPhase, CNode *msgPeriodicNode);
        void computeScheduleExtensions(TTime lengthPeriodicPhase, 
            uint slotTableSize, TTime *shiftStartTransPhase, 
            TTime *shiftEndTransPhase, TTime *duplicationPeriodicPhase);
        CNode *traceConstructMessages(uint slotTableSize, 
            TTime lengthPeriodicPhase);
        
        // Output actor
        void initOutputActor();

        // Sanity checks on the binding-aware SDFG
        void checkBindingAwareSDFG();
        
        // Platform graph
        PlatformGraph *platformGraph;
        
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
        
        /***********************************************************************
         * Tracing
         **********************************************************************/
        // Flag indicating wether tokens on channel are traced
        bool *isChannelTraced;

        // Flag indicating wether periodic state is found
        bool foundPeriodicState;

        // Periodic state
        State periodicState;

        // Source/destination tile of a channel
        uint *srcTileCh;
        uint *dstTileCh;

        // Internal clock of each tile
        TTime *tileClock;

        /**
         * Token
         * Container for traced token information.
         */
        typedef struct _Token
        {
            // Token identifiers
            CId id;
            CId seqNr;
            CId channelId;

            // Timing
            TTime prodTime;
            TTime consTime;

            // Next token / next token in channel
            struct _Token *next;
            struct _Token *nextInChannel;

            // Last stored state before token production
            State state;

            // Token belongs to periodic phase
            bool inPeriodicPhase;
        } Token;

        // Linked list of all tokens traced in SDFG execution
        Token *tokens;       // Head (token inserted on this side)
        Token *firstToken;   // Tail (first token ever)

        // Pointer to the last token which is produced, or NULL when
        // no token is produced on the channel yet.
        Token **lastProdTokenCh;

        // Pointer to the next token which is must be consumed, or NULL when
        // no token is produced on the channel yet.
        Token **nextConsTokenCh;

        // Pointer to first token in channel
        Token **firstTokenCh;
    };
};

#endif
