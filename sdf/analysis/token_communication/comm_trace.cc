/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   comm_trace.cc
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
 * $Id: comm_trace.cc,v 1.1 2008/03/06 10:49:44 sander Exp $
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

#include "comm_trace.h"
#include "../../base/algo/repetition_vector.h"
#include "../../base/algo/components.h"

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * clear ()
 * The function sets the state to zero.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem::State
    ::clear()
{
    for (uint i = 0; i < actClk.size(); i++)
    {
        actClk[i].clear();
    }

    for (uint i = 0; i < ch.size(); i++)
    {
        ch[i] = 0;
    }
    
    glbClk = 0;
    
    for (uint i = 0; i < schedulePos.size(); i++)
    {
        schedulePos[i] = 0;
        tdmaPos[i] = 0;
    }
}

/**
 * operator= ()
 * The function compares to states and returns true if they are equal.
 */
bool SDFstateSpaceTraceInterconnectCommunication::TransitionSystem::State
    ::operator==(const State &s)
{
    if (glbClk != s.glbClk)
        return false;
    
    for (uint i = 0; i < ch.size(); i++)
    {
        if (ch[i] != s.ch[i])
            return false;
    }
    
    for (uint i = 0; i < actClk.size(); i++)
    {
        if (actClk[i] != s.actClk[i])
            return false;
    }

    for (uint i = 0; i < schedulePos.size(); i++)
    {
        if (schedulePos[i] != s.schedulePos[i] || tdmaPos[i] != s.tdmaPos[i])
            return false;
    }
    
    return true;
}

/**
 * print ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem::State
        ::print(ostream &out)
{
    out << "### State ###" << endl;

    for (uint i = 0; i < actClk.size(); i++)
    {
        out << "actClk[" << i << "] =";
        
        for (list<SDFtime>::const_iterator iter = actClk[i].begin();
                iter != actClk[i].end(); iter++)
        {
            out << " " << (*iter) << ", ";
        }
        
        out << endl;
    }

    for (uint i = 0; i < ch.size(); i++)
    {
        out << "ch[" << i << "] = " << ch[i] << endl;
    }

    out << "glbClk = " << glbClk << endl;
    
    for (uint i = 0; i < schedulePos.size(); i++)
    {
        out << "tile[" << i << "] = (";
        out << schedulePos[i];
        out << ", " << tdmaPos[i] << ")" << endl;
    }
}

/******************************************************************************
 * Transition system
 *****************************************************************************/

/**
 * initOutputActor ()
 * The function selects an actor to be used as output actor in the
 * state transition system.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::initOutputActor()
{
    RepetitionVector repVec;
    int min = INT_MAX;
    SDFactor *a = NULL;

    // Compute repetition vector
    repVec = computeRepetitionVector(bindingAwareSDFG);
    
    // Select actor with lowest entry in repetition vector as output actor
    for (SDFactorsIter iter = bindingAwareSDFG->actorsBegin();
            iter != bindingAwareSDFG->actorsEnd(); iter++)
    {
        if (repVec[(*iter)->getId()] < min)
        {
            a = *iter;
            min = repVec[a->getId()];
        }
    }
    
    // Set output actor and its repetition vector count
    outputActor = a;
    outputActorRepCnt = repVec[outputActor->getId()];
}

/**
 * checkBindingAwareSDFG ()
 * The function performs some sanity checks on the binding-aware SDFG
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::checkBindingAwareSDFG()
{
    CId tileId;
    
    // Check that all actor that are bound to a processors are bound to a 
    // processor which has a schedule
    for (SDFactorsIter iter = bindingAwareSDFG->actorsBegin();
            iter != bindingAwareSDFG->actorsEnd(); iter++)
    {
        tileId = bindingAwareSDFG->getBindingOfActorToTile(*iter);
        
        if (tileId != ACTOR_NOT_BOUND)
        {
            if (bindingAwareSDFG->getScheduleOnTile(tileId).empty())
            {
                throw CException("Actor mapped to processor without schedule.");
            }
        }
    }
}

/**
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the state s is not stored. The function
 * returns false. The function always sets the pos variable to the position
 * where the state s is in the list.
 */
bool SDFstateSpaceTraceInterconnectCommunication::TransitionSystem::storeState(
        State &s, StatesIter &pos)
{
    // Find state in the list of stored states
    for (StatesIter iter = storedStates.begin();
            iter != storedStates.end(); iter++)
    {
        State &x = *iter;
        
        // State s at position iter in the list?
        if (x == s)
        {
            pos = iter;
            return false;
        }
    }
    
    // State not found, store it at the end of the list
    storedStates.push_back(s);
    
    // Added state to the end of the list
    pos = storedStates.end();
    
    return true;
}

/******************************************************************************
 * Tracing
 *****************************************************************************/

/**
 * ~TransitionSystem ()
 * Destructor.
 */
SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::~TransitionSystem()
{
    Token *t;
    
    // Delete all tokens that have been traced
    while (tokens != NULL)
    {
        t = tokens;
        tokens = tokens->next;
        delete t;
    }
    
    // Delete vectors that store status information of the trace
    delete [] lastProdTokenCh;
    delete [] nextConsTokenCh;
    delete [] firstTokenCh;
    delete [] srcTileCh;
    delete [] dstTileCh;
    delete [] isChannelTraced;
    delete [] tileClock;
}

/**
 * initTracing ()
 * Initialize all data structures needed for the token tracing.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::initTracing()
{
    // Init globals
    foundPeriodicState = false;
    tokens = NULL;
    firstToken = NULL;

    // Allocate memory for all data-structures
    lastProdTokenCh = new Token* [bindingAwareSDFG->nrChannels()];
    nextConsTokenCh = new Token* [bindingAwareSDFG->nrChannels()];
    firstTokenCh = new Token* [bindingAwareSDFG->nrChannels()];
    srcTileCh = new uint [bindingAwareSDFG->nrChannels()];
    dstTileCh = new uint [bindingAwareSDFG->nrChannels()];
    isChannelTraced = new bool [bindingAwareSDFG->nrChannels()];
    tileClock = new TTime [bindingAwareSDFG->nrTilesInPlatformGraph()];

    // Initialize periodic state
    periodicState.init(bindingAwareSDFG->nrActors(), 
                        bindingAwareSDFG->nrChannels(), 
                        bindingAwareSDFG->nrTilesInPlatformGraph());

    // No tokens produced or consumed yet
    for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++)
    {
        lastProdTokenCh[i] = NULL;
        nextConsTokenCh[i] = NULL;
        firstTokenCh[i] = NULL;
    }

    // Iterate over all channels
    for (SDFchannelsIter iter = bindingAwareSDFG->channelsBegin();
            iter != bindingAwareSDFG->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        SDFactor *srcActor = c->getSrcActor();
        SDFactor *dstActor = c->getDstActor();
        uint srcTile = bindingAwareSDFG->getBindingOfActorToTile(srcActor);
        uint dstTile = bindingAwareSDFG->getBindingOfActorToTile(dstActor);
        
        // Is channel bound to the interconnect?
        if (srcTile != ACTOR_NOT_BOUND && dstTile != ACTOR_NOT_BOUND
                && srcTile != dstTile)
        {
            srcTileCh[c->getId()] = srcTile;
            dstTileCh[c->getId()] = dstTile;
            isChannelTraced[c->getId()] = true;
        }
        else
        {
            isChannelTraced[c->getId()] = false;
        }
    }
    
    // Its the beginning of all time...
    for (uint i = 0; i < bindingAwareSDFG->nrTilesInPlatformGraph(); i++)
    {
        tileClock[i] = 0;
    }
}

/**
 * traceConsumptionToken ()
 * Trace the consumption of a token on the given channel.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::traceConsumptionToken(const CId &ch, const uint &rate,
            const SDFtime &delay)
{
    // Is this channel traced?
    if (!isChannelTraced[ch])
        return;
    
    // Consumed all tokens from the channel?
    if (nextConsTokenCh[ch] == NULL)
        return;
    
    // Consume 'rate' tokens
    for (uint i = 0; i < rate; i++)
    {
        // Token consumed at current destination tile clock plus delay
        // The delay takes into account that actor in state-space can be
        // fired before start of time slice is reached. Delay gives
        // time difference between both moments in time.
        nextConsTokenCh[ch]->consTime = tileClock[dstTileCh[ch]] + delay;

        // Move to next token for consumption
        nextConsTokenCh[ch] = nextConsTokenCh[ch]->nextInChannel;

        // Consumed all (traced) tokens from the channel?
        if (nextConsTokenCh[ch] == NULL)
            return;
    }
}

/**
 * traceProductionToken ()
 * Trace the production of a token on the given channel.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::traceProductionToken(const CId &ch, const uint &rate)
{
    Token *t;
    
    // Is this channel traced?
    if (!isChannelTraced[ch])
        return;
    
    // Stop tracing production of tokens when periodic state is found.
    // Is this state already discovered?
    if (foundPeriodicState)
        return;
    
    // Produce 'rate' tokens
    for (uint i = 0; i < rate; i++)
    {
        // Allocate memory for new token
        t = new Token;
        t->next = NULL;
        
        // Token produced at current source tile clock
        t->prodTime = tileClock[srcTileCh[ch]];

        // Token is the newest token in channel
        t->channelId = ch;
        t->nextInChannel = NULL;

        // Associate last stored state with the token
        if (storedStates.empty())
        {
            t->state = State(bindingAwareSDFG->nrActors(),
                                bindingAwareSDFG->nrChannels(), 
                                bindingAwareSDFG->nrTilesInPlatformGraph());
            t->state.clear();
        }
        else
        {
            t->state = storedStates.back();
        }
        t->inPeriodicPhase = false;

        // Is this the first token ever in the channel?
        if (lastProdTokenCh[ch] == NULL)
        {
            t->seqNr = 0;
            firstTokenCh[ch] = t;
        }
        else
        {
            t->seqNr = lastProdTokenCh[ch]->seqNr + 1;
            lastProdTokenCh[ch]->nextInChannel = t;
        }

        // Is this the first token ever on any channel?
        if (tokens == NULL)
        {
            t->id = 0;
            t->next = NULL;
            firstToken = t;
        }
        else
        {
            t->id = tokens->id + 1;
            t->next = tokens;
        }

        // Was channel empty when this token is produced?
        if (nextConsTokenCh[ch] == NULL)
            nextConsTokenCh[ch] = t;

        // Produce token on channel and on list of all tokens
        lastProdTokenCh[ch] = t;
        tokens = t;
    }
}

/**
 * traceAdvanceClocks ()
 * The function increases the clocks of all tiles with one time unit.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::traceAdvanceClocks(const SDFtime step)
{
    for (uint i = 0; i < bindingAwareSDFG->nrTilesInPlatformGraph(); i++)
        tileClock[i] += step;
}

/**
 * isTracingCompleted ()
 * The function checks wether the tracing of all tokens is completed. On
 * completion, it returns true. Else it returns false. The tracing is completed
 * once the periodic state is found and all tokens produced till this state is
 * found are also consumed.
 */
bool SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::isTracingCompleted()
{
    // Periodic state not found yet?
    if (!foundPeriodicState)
        return false;
    
    for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++)
    {
        // Tokens left to consume?
        if (nextConsTokenCh[i] != NULL)
            return false;
    }
    
    return true;
}

/**
 * traceMarkTokensInPeriodicPhase ()
 * The function must be called after the SDFG is executed. It marks all tokens
 * as beloning to the periodic phase or not.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::traceMarkTokensInPeriodicPhase()
{
    Token *t = firstToken;
    bool periodic = false;
    
    while (t != NULL)
    {
        // State in which token was stored equal to periodic state?
        if (t->state == periodicState)
        {
            periodic = true;
        }
        
        // Mark token
        t->inPeriodicPhase = periodic;
        
        // Next
        t = t->next;
    }
}

/**
 * traceMessagesTransient ()
 * The function adds a list of all messages (tokens) sent in the transient phase
 * of the SDFG execution to the msgTransNode. It returns the period (length) of
 * the transient phase.
 */
TTime SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::traceMessagesTransient(TTime shiftStartTransPhase, 
        TTime shiftEndTransPhase, CNode *msgTransNode)
{
    CNode *msgNode;
    Token *t;
    TTime period = 0;
    
    // Iterate over all channels
    for (SDFchannelsIter iter = bindingAwareSDFG->channelsBegin();
            iter != bindingAwareSDFG->channelsEnd(); iter++)
    {
        TimedSDFchannel *ch = (TimedSDFchannel*)(*iter);
        
        // Is this channel not traced?
        if (!isChannelTraced[ch->getId()])
            continue;
        
        // Iterate over the tokens in the channel till first token produced in
        // the periodic state is encountered.
        t = firstTokenCh[ch->getId()];
        while (t != NULL && t->inPeriodicPhase == false)
        {
            Tile *srcTile = platformGraph->getTile(srcTileCh[ch->getId()]);
            Tile *dstTile = platformGraph->getTile(dstTileCh[ch->getId()]);
            Processor *p = dstTile->getProcessor();
            
            // Update consumption time for required synchronization
            // overhead on destination tile
            t->consTime -= (p->getTimewheelSize() - p->getReservedTimeSlice());
            
            // Add token to list of messages
            msgNode = CAddNode(msgTransNode, "message");
            CAddAttribute(msgNode, "nr", t->id);
            CAddAttribute(msgNode, "src", srcTile->getName());
            CAddAttribute(msgNode, "dst", dstTile->getName());
            CAddAttribute(msgNode, "channel", ch->getName());
            CAddAttribute(msgNode, "seqNr", t->seqNr);
            CAddAttribute(msgNode, "startTime", 
                                        t->prodTime + shiftStartTransPhase);
            CAddAttribute(msgNode, "duration", t->consTime - t->prodTime);
            CAddAttribute(msgNode, "size", ch->getTokenSize());
            
            // Period is equal to largest consumption time plus one
            if (t->consTime + 1 > period)
                period = t->consTime + 1;
            
            // Next token
            t = t->nextInChannel;
        }
    }
    
    return period + shiftStartTransPhase + shiftEndTransPhase;
}

/**
 * traceMessagesPeriodic ()
 * The function adds a list of all messages (tokens) sent in the periodic phase
 * of the SDFG execution to the msgPeriodicNode. The function returns the
 * overlap between the periodic and transient schedule.
 */
TTime SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::traceMessagesPeriodic(uint duplicationPeriodicPhase, 
        TTime lengthPeriodicPhase, CNode *msgPeriodicNode)
{
    TTime startPeriod, overlap, endTransient = 0;
    TTime prodTime, consTime;
    CNode *msgNode;
    uint *seqNrCh;
    Token *t;
    CId msgId = 0;
    
    // Initialize sequence numbers for messages in channels
    seqNrCh = new uint [bindingAwareSDFG->nrChannels()];
    for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++)
        seqNrCh[i] = 0;
    
    // Starting point of periodic phase is its first production
    t = firstToken;
    while (t != NULL && t->inPeriodicPhase == false)
    {
        t = t->next;
    }
    if (t == NULL)
        return 0;
    startPeriod = t->prodTime;
    
    // Iterate over all channels
    for (SDFchannelsIter iter = bindingAwareSDFG->channelsBegin();
            iter != bindingAwareSDFG->channelsEnd(); iter++)
    {
        TimedSDFchannel *ch = (TimedSDFchannel*)(*iter);
        
        // Is this channel not traced?
        if (!isChannelTraced[ch->getId()])
            continue;
        
        for (uint d = 0; d < duplicationPeriodicPhase; d++)
        {
            // Iterate over the tokens in the channel
            t = firstTokenCh[ch->getId()];
            while (t != NULL)
            {
                // Token in periodic phase?
                if (t->inPeriodicPhase)
                {
                    Tile *srcTile = platformGraph->getTile(srcTileCh[ch->getId()]);
                    Tile *dstTile = platformGraph->getTile(dstTileCh[ch->getId()]);
                    Processor *p = dstTile->getProcessor();

                    // Update production time (offset from transient)
                    prodTime = t->prodTime - startPeriod 
                               + d * lengthPeriodicPhase;

                    // Update consumption time for required synchronization
                    // overhead on destination tile and offset transient
                    consTime = t->consTime - startPeriod 
                                  - (p->getTimewheelSize() 
                                  - p->getReservedTimeSlice())
                                  + d * lengthPeriodicPhase;

                    // Add token to list of messages
                    msgNode = CAddNode(msgPeriodicNode, "message");
                    CAddAttribute(msgNode, "nr", msgId);
                    CAddAttribute(msgNode, "src", srcTile->getName());
                    CAddAttribute(msgNode, "dst", dstTile->getName());
                    CAddAttribute(msgNode, "channel", ch->getName());
                    CAddAttribute(msgNode, "seqNr", seqNrCh[ch->getId()]);
                    CAddAttribute(msgNode, "startTime", prodTime);
                    CAddAttribute(msgNode, "duration", consTime - prodTime);
                    CAddAttribute(msgNode, "size", ch->getTokenSize());
                
                    // Increase sequence number of messages in channel ch
                    seqNrCh[ch->getId()]++;
                    
                    // Increase message id
                    msgId++;
                }
                else
                {
                    if (t->consTime > endTransient)
                        endTransient = t->consTime;
                }

                // Next token
                t = t->nextInChannel;
            }
        }
    }

    // Overlap is equal to the latest consumption in transient phase
    // minus first production in periodic phase
    if (endTransient < startPeriod)
        overlap = 0;
    else
        overlap = endTransient - startPeriod + 1;
    
    // Cleanup
    delete [] seqNrCh;
    
    return overlap;
}

/**
 * computeScheduleExtensions ()
 * The periodic behavior with which messages are communicated may not be a
 * multiple of the slot-table size. The same may hold for the overlap between
 * the transient and periodic phase. The NoC scheduling algorithm requires that
 * both the periods and the overlap are a multiple of the slot table. This is
 * needed to guarantee a contention-free schedule of the messages. This
 * requirement can be met by shifting the start and end time of the transient
 * schedule and duplicating the periodic schedule for a integer number of times.
 * The function computes the required extensions and duplication.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::computeScheduleExtensions(TTime lengthPeriodicPhase, uint slotTableSize,
        TTime *shiftStartTransPhase, TTime *shiftEndTransPhase, 
        TTime *duplicationPeriodicPhase)
{
    TTime consTime, overlap, lengthTransientPhase = 0;
    Token *t;

    // Compute length transient schedule
    for (SDFchannelsIter iter = bindingAwareSDFG->channelsBegin();
            iter != bindingAwareSDFG->channelsEnd(); iter++)
    {
        TimedSDFchannel *ch = (TimedSDFchannel*)(*iter);
        
        // Is this channel not traced?
        if (!isChannelTraced[ch->getId()])
            continue;
        
        // Iterate over the tokens in the channel till first token produced in
        // the periodic state is encountered.
        t = firstTokenCh[ch->getId()];
        while (t != NULL && t->inPeriodicPhase == false)
        {
            Tile *dstTile = platformGraph->getTile(dstTileCh[ch->getId()]);
            Processor *p = dstTile->getProcessor();
            
            // Consumption time corrected for required synchronization
            // overhead on destination tile
            consTime = t->consTime;
            consTime -= (p->getTimewheelSize() - p->getReservedTimeSlice());
            
            // Period is equal to largest consumption time plus one
            if (consTime + 1 > lengthTransientPhase)
                lengthTransientPhase = consTime + 1;
            
            // Next token
            t = t->nextInChannel;
        }
    }
    
    // Compute overlap between transient and periodic schedule
    // Starting point of periodic phase is its first production
    t = firstToken;
    while (t != NULL && t->inPeriodicPhase == false) { t = t->next; }
    if (t == NULL)
    {
        overlap = 0;
    }
    else
    {
        // Overlap is equal to the latest consumption in transient phase
        // minus first production in periodic phase
        if (lengthTransientPhase < t->prodTime)
            overlap = 0;
        else
            overlap = lengthTransientPhase - t->prodTime;
    }

    // Shifts and overlap...
    *shiftEndTransPhase = (slotTableSize - (overlap % slotTableSize)) 
                                                        % slotTableSize;
    
    *shiftStartTransPhase = (slotTableSize - ((lengthTransientPhase +
                        *shiftEndTransPhase) % slotTableSize)) % slotTableSize;
    
    *duplicationPeriodicPhase = lcm(lengthPeriodicPhase, (TTime)slotTableSize)
                                                        / lengthPeriodicPhase;
}

/**
 * traceConstructMessages ()
 * The function returns two sets of messages. One set contains all tokens
 * (messages) which belong to the transient part of the execution. The other set
 * contains all tokens which belong to the periodic part of the execution.
 */
CNode *SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::traceConstructMessages(uint slotTableSize, TTime lengthPeriodicPhase)
{
    TTime shiftStartTransPhase, shiftEndTransPhase, duplicationPeriodicPhase;
    CNode *messagesSetNode, *mgsTransNode, *msgPeriodicNode;
    CNode *switchNode;
    TTime lengthTransientPhase, overlap;
    CString nmPeriod, nmTrans;
    
    // Mark tokens as transient or periodic
    traceMarkTokensInPeriodicPhase();

    // Compute schedule extensions
    computeScheduleExtensions(lengthPeriodicPhase, slotTableSize,
        &shiftStartTransPhase, &shiftEndTransPhase, &duplicationPeriodicPhase);

    // Message sets
    messagesSetNode = CNewNode("messagesSet");
    
    // Transient messages
    nmTrans = bindingAwareSDFG->getName() + "_transient";
    mgsTransNode = CAddNode(messagesSetNode, "messages");
    CAddAttribute(mgsTransNode, "name", nmTrans);
    lengthTransientPhase = traceMessagesTransient(shiftStartTransPhase,
        shiftEndTransPhase, mgsTransNode);
    CAddAttribute(mgsTransNode, "period", lengthTransientPhase);

    // Periodic messages
    nmPeriod = bindingAwareSDFG->getName() + "_periodic";
    msgPeriodicNode = CAddNode(messagesSetNode, "messages");
    CAddAttribute(msgPeriodicNode, "name", nmPeriod);
    overlap = traceMessagesPeriodic(duplicationPeriodicPhase, 
                                        lengthPeriodicPhase, msgPeriodicNode);
    CAddAttribute(msgPeriodicNode, "period",
                                lengthPeriodicPhase * duplicationPeriodicPhase);

    // Switch
    switchNode = CAddNode(messagesSetNode, "switch");
    CAddAttribute(switchNode, "from", nmTrans);
    CAddAttribute(switchNode, "to", nmPeriod);
    CAddAttribute(switchNode, "overlap", overlap + shiftEndTransPhase);
    
    return messagesSetNode;
}

/******************************************************************************
 * SDF
 *****************************************************************************/

#define CH(c)               currentState.ch[c]
#define TDMA_POS(p)         currentState.tdmaPos[p]
#define SOS_POS(p)          currentState.schedulePos[p]

#define SOS(p)              (bindingAwareSDFG->getScheduleOnTile(p))
#define SOS_ENTRY(p)        (SOS(p).getScheduleEntry(SOS_POS(p)))
#define SOS_NEXT_POS(p)     (SOS(p).next(SOS_POS(p)))

#define CH_TOKENS(c,n)      (CH(c) >= n)  
#define CONSUME(c,n)        CH(c) = CH(c) - n;
#define PRODUCE(c,n)        CH(c) = CH(c) + n;

#define CH_TOKENS_PREV(c,n) (previousState.ch[c] >= n)  

/**  
 * computeLengthPeriodicPhase ()  
 * The function returns the length (in time-units) of the periodic phase.
 */  
TTime SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::computeLengthPeriodicPhase(const StatesIter cycleIter)  
{  
	TTime time = 0;  

	// Check all state from stack till cycle complete  
	for (StatesIter iter = cycleIter;
            iter != storedStates.end(); iter++)
    {
        State &s = *iter;

	    // Time between previous state  
	    time += s.glbClk;
	}

	return time;
}

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::actorReadyToFire(SDFactor *a)
{
    // Actor bound to processor?
    if (bindingAwareSDFG->getBindingOfActorToTile(a) != ACTOR_NOT_BOUND)
    {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(a);
        
        // Actor not scheduled on processor?
        if (SOS_ENTRY(p)->actor->getId() != a->getId())
            return false;
    }
    
    // Check all input ports for tokens
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        
        // Actor is destination of the channel?
        if (p->getType() == SDFport::In)
        {
            if (!CH_TOKENS(c->getId(), p->getRate()))
            {
                return false;
            }    
        }
    }

    return true;
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::startActorFiring(TimedSDFactor *a)
{
    SDFtime execTime, completionTime, waitingTime, timeTileStartOfSlice = 0;
    int remainingExecTime, nrOfFullRotationsInNonReservedPart;
    
    // Consume tokens from inputs and space for output tokens
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        
        // Actor is destination of the channel?
        if (p->getType() == SDFport::In)
        {
            CONSUME(c->getId(), p->getRate());
        }
    }

    // Execution time of the actor
    execTime = a->getExecutionTime();
    
    // Compute time needed to complete actor firing
    if (bindingAwareSDFG->getBindingOfActorToTile(a) == ACTOR_NOT_BOUND)
    {
        completionTime = execTime;
    }
    else
    {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(a);
         
        // Time wheel has not yet reached start of slice?
        if (currentState.tdmaPos[p] 
                < bindingAwareSDFG->getTDMAsizeOnTile(p) 
                        - bindingAwareSDFG->getTDMAsliceOnTile(p))
        {
            // Wait till start of slice and next complete the number
            // of required wheel rotations to get the full execution time
            // of the slice
            timeTileStartOfSlice = bindingAwareSDFG->getTDMAsizeOnTile(p) 
                          - bindingAwareSDFG->getTDMAsliceOnTile(p) 
                          - currentState.tdmaPos[p];
            nrOfFullRotationsInNonReservedPart = (int) ceil((double) execTime 
                          / bindingAwareSDFG->getTDMAsliceOnTile(p)) - 1;
            waitingTime = (bindingAwareSDFG->getTDMAsizeOnTile(p) 
                          - bindingAwareSDFG->getTDMAsliceOnTile(p))
                            * nrOfFullRotationsInNonReservedPart;
            completionTime = timeTileStartOfSlice + execTime + waitingTime;
        }
        else
        {
            // Start the execution time immediatly, wait the number of wheel
            // rotations after the current slice has been used to complete the
            // firing
            remainingExecTime = (int)execTime 
                                - (int)bindingAwareSDFG->getTDMAsizeOnTile(p) 
                                + (int)currentState.tdmaPos[p];
            
            if (remainingExecTime < 0)
            {
                waitingTime = 0;
            }
            else
            {
                waitingTime = (remainingExecTime 
                                    / bindingAwareSDFG->getTDMAsliceOnTile(p)) 
                            * (bindingAwareSDFG->getTDMAsizeOnTile(p) 
                                    - bindingAwareSDFG->getTDMAsliceOnTile(p));
            }
     
            completionTime = execTime + waitingTime;
        }
    }

    // Add actor firing to the list of active firings of this actor
    currentState.actClk[a->getId()].push_back(completionTime);
    
    // Trace consumption of tokens; This takes place at the start of the firing
    // which is the first point in time after the timeTileStartOfSlice has
    // elapsed.
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        
        // Actor is destination of the channel?
        if (p->getType() == SDFport::In)
        {
            traceConsumptionToken(c->getId(), p->getRate(),
                                            timeTileStartOfSlice);
        }
    }
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::actorReadyToEnd(SDFactor *a)
{
    if (currentState.actClk[a->getId()].empty())
        return false;
    
    // First actor firing in sorted list has execution time left?
    if (currentState.actClk[a->getId()].front() != 0)
        return false;

    return true;
}

/**
 * endActorFiring ()
 * Produce tokens on all output channels and remove the actor firing from the
 * list of active firings.
 */
void SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::endActorFiring(SDFactor *a)
{
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        
        // Actor is source of the channel?
        if (p->getType() == SDFport::Out)
        {
            PRODUCE(c->getId(), p->getRate());
            traceProductionToken(c->getId(), p->getRate());
        }
    }

    // Remove the firing from the list of active actor firings
    currentState.actClk[a->getId()].pop_front();

    // Actor bound to processor?
    if (bindingAwareSDFG->getBindingOfActorToTile(a) != ACTOR_NOT_BOUND )
    {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(a);

        // Advance the schedule to the next state
        SOS_POS(p) = SOS_NEXT_POS(p);
    }    
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
SDFtime SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::clockStep()
{
    SDFtime step = UINT_MAX;
    
    // Find maximal time progress
    for (uint a = 0; a < bindingAwareSDFG->nrActors(); a++)
    {
        if (!currentState.actClk[a].empty())
        {
            SDFtime actClk = currentState.actClk[a].front();

            if (step > actClk)
                step = actClk;
        }
    }

    // Still actors ready to end their firing?
    if (step == 0)
        return 0;

	// Check for progress (i.e. no deadlock) 
	if (step == UINT_MAX)
        return UINT_MAX;

    // Lower remaining execution time actors
    for (uint a = 0; a < bindingAwareSDFG->nrActors(); a++)
    {
        for (list<SDFtime>::iterator iter = currentState.actClk[a].begin();
                    iter != currentState.actClk[a].end(); iter++)
        {
            SDFtime &actFiringTime = *iter;
            
            // Lower remaining execution time of the actor firing
            actFiringTime -= step;
        }
    }

    // Advance the time wheels
    for (uint t = 0; t < bindingAwareSDFG->nrTilesInPlatformGraph(); t++)
    {
        currentState.tdmaPos[t] = (currentState.tdmaPos[t] + step) 
                                    % bindingAwareSDFG->getTDMAsizeOnTile(t);
    }

    // Advance the global clock  
    currentState.glbClk += step;

    // Advance the clocks in the tracing mechanism
    traceAdvanceClocks(step);

    return step;
}

/**  
 * execSDFgraph()  
 * Execute the SDF graph till a deadlock is found or a recurrent state.  
 * The length of the periodic phase is returned.  
 */  
TTime SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::execSDFgraph()  
{
    StatesIter recurrentState;
    TTime lengthPeriodicState = 0;
    SDFtime clkStep;
    int repCnt = 0;  

    // Clear the list of stored states
    clearStoredStates();
    
    // Create initial state
    currentState.init(bindingAwareSDFG->nrActors(),
                        bindingAwareSDFG->nrChannels(),
                        bindingAwareSDFG->nrTilesInPlatformGraph());
    currentState.clear();  
    previousState.init(bindingAwareSDFG->nrActors(),
                        bindingAwareSDFG->nrChannels(),
                        bindingAwareSDFG->nrTilesInPlatformGraph());
    previousState.clear();

    // Initial tokens
    for (SDFchannelsIter iter = bindingAwareSDFG->channelsBegin();
            iter != bindingAwareSDFG->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        
        CH(c->getId()) = c->getInitialTokens();
    }
    
    // Initial schedules
    for (uint p = 0; p < bindingAwareSDFG->nrTilesInPlatformGraph(); p++)
    {
        SOS_POS(p) = 0;
        TDMA_POS(p) = 0;
    }

    // Fire the actors  
    while (true)  
    {
        // Tracing compeleted?
        if (isTracingCompleted())
            return lengthPeriodicState;

        // Store partial state to check for progress
        for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++)
        {
            previousState.ch[i] = currentState.ch[i];
        }

        // Finish actor firings  
        for (SDFactorsIter iter = bindingAwareSDFG->actorsBegin();
                iter != bindingAwareSDFG->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            
            while (actorReadyToEnd(a))  
            {
                if (outputActor->getId() == a->getId())
                {
                    repCnt++;
                    if (repCnt == outputActorRepCnt)  
                    { 
                        // Add state to hash of visited states  
                        if (!storeState(currentState, recurrentState))
                        {
                            foundPeriodicState = true;
                            periodicState = currentState;
                            lengthPeriodicState 
                                = computeLengthPeriodicPhase(recurrentState);
                        }
	                    currentState.glbClk = 0;
                        repCnt = 0;
                    }  
                }

                // End the actor firing
                endActorFiring(a);
            }  
        }

        // Start actor firings  
        for (SDFactorsIter iter = bindingAwareSDFG->actorsBegin();
                iter != bindingAwareSDFG->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)(*iter);
        
            // Ready to fire actor a?
            while (actorReadyToFire(a))
            {
                // Fire actor a
                startActorFiring(a);
            }
        }

        // Clock step
        clkStep = clockStep();

        // Deadlocked?
        if (clkStep == UINT_MAX)
        {
            return 0;
        }
    }
    
    return 0;
}  

/**
 * traceCommunication ()
 * The function generates a trace of all communication over the interconnect
 * of a mapped and scheduled SDFG on a NoC-based platform. The relation 
 * between the communication in the transient and periodic part of the
 * schedule is also included.
 */
CNode *SDFstateSpaceTraceInterconnectCommunication::TransitionSystem
    ::traceCommunication(uint slotTableSize)
{
    CNode *messagesSetNode;
    TTime lengthPeriodicPhase;

    // Execute the graph...
    lengthPeriodicPhase = execSDFgraph();

    // Construct the actual trace of the tokens (messages)
    messagesSetNode = traceConstructMessages(slotTableSize,lengthPeriodicPhase);

    return messagesSetNode;
}

/**
 * Trace interconnect communication
 * Generates a trace of all communication over the interconnect
 * of a mapped and scheduled SDFG on a NoC-based platform. The relation 
 * between the communication in the transient and periodic part of the
 * schedule is also included.
 */
CNode *SDFstateSpaceTraceInterconnectCommunication::trace(BindingAwareSDFG *bg, 
        PlatformGraph *pg, uint slotTableSize)
{
    CNode *messagesSetNode;

    // Check that the binding-aware graph is a strongly connected graph
    if (!isStronglyConnectedGraph(bg))
        throw CException("Graph is not strongly connected.");

    // Create a transition system
    TransitionSystem transitionSystem(bg, pg);

    // Trace the interconnect communication
    messagesSetNode = transitionSystem.traceCommunication(slotTableSize);

    return  messagesSetNode;
}

