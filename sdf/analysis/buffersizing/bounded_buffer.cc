/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   bounded_buffer.cc
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
 * $Id: bounded_buffer.cc,v 1.1 2008/03/06 10:49:42 sander Exp $
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

#include "bounded_buffer.h"
#include "../../base/algo/repetition_vector.h"

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

/******************************************************************************
 * Bounds on the search space
 *****************************************************************************/

/**
 * initBoundsSearchSpace ()
 * Compute bounds on the trade-off space that must be explored.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::initBoundsSearchSpace(
        TimedSDFgraph *g, bool *bufferChannels)
{
    initMinimalChannelSzStep(g);
    initMinimalChannelSz(g, bufferChannels);
    initLbDistributionSz(g);
    initMaxThroughput(g);
}

/**
 * initMinimalChannelSzStep ()
 * Compute lower bound on the step size of channels
 */
void SDFstateSpaceBindingAwareBufferAnalysis::initMinimalChannelSzStep(
        TimedSDFgraph *g)
{
    minSzStep = new TBufSize [g->nrChannels()];
    
    for (SDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        SDFchannel *ch = *iter;
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint minStepSz;
        
        minStepSz = gcd(p,c);

        minSzStep[ch->getId()] = minStepSz;    
    }
}

/**
 * initMinimalChannelSz ()
 * Compute lower bound on the buffer size needed for positive throughput
 */
void SDFstateSpaceBindingAwareBufferAnalysis::initMinimalChannelSz(
        TimedSDFgraph *g, bool *bufferChannels)
{
    minSz =  new TBufSize [g->nrChannels()];    

    for (SDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        SDFchannel *ch = *iter;
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint t = ch->getInitialTokens();
        uint lb;
        
        // Channel models buffer space (ignore initial tokens)?
        if (bufferChannels[ch->getId()])
        {
            lb = p+c-gcd(p,c)+t%gcd(p,c);

            minSz[ch->getId()] = lb;    
        }
        else
        {
            lb = p+c-gcd(p,c)+t%gcd(p,c);
            lb = (lb > t ? lb : t);

            minSz[ch->getId()] = lb;    
        }

        // Lower-bound of a self-edge is rate at which data is produced and
        // consumed
        if (ch->getSrcActor()->getId() == ch->getDstActor()->getId())
            minSz[ch->getId()] = p + c;
    }
}

/**
 * initLbDistributionSz ()
 * Compute lower bound on the buffer size needed for positive throughput
 */
void SDFstateSpaceBindingAwareBufferAnalysis::initLbDistributionSz(
        TimedSDFgraph *g)
{
    lbDistributionSz = 0;
    
    for (uint c = 0; c < g->nrChannels(); c++)
        lbDistributionSz += minSz[c];
}

/**
 * initMaxThroughput ()
 * Compute the maximal throughput that can be achieved by the graph.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::initMaxThroughput(
        TimedSDFgraph *g)
{
    maxThroughput = TDTIME_MAX;
}

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * clear ()
 * The function sets the state to zero.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::State
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
bool SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::State
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
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::State
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
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
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
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
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
bool SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::storeState(
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

/**  
 * computeThroughput ()  
 * The function calculates the throughput of the states on the cycle. Its  
 * value is equal to the average number of firings of an actor per time unit.
 */  
TDtime SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
    ::computeThroughput(const StatesIter cycleIter)  
{  
	int nr_fire = 0;
	TDtime time = 0;  

	// Check all state from stack till cycle complete  
	for (StatesIter iter = cycleIter; iter != storedStates.end(); iter++)
    {
        State &s = *iter;

        // Number of states in cycle is equal to number of iterations 
        // in the period
        nr_fire++;

	    // Time between previous state  
	    time += s.glbClk;
	}

	return (TDtime)(nr_fire)/(time);  
}

/******************************************************************************
 * Dependencies
 *****************************************************************************/

/**
 * dfsVisitDependencies ()
 * The function performs a DFS on the abstract dependency graph from a node a to
 * find all cycles of which a is part. Channels on a cycle are when needed
 * marked to have a storage dependency.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
    ::dfsVisitDependencies(uint a, int *color, int *pi, bool **abstractDepGraph,
            bool *dep)
{
    uint c, d;

    // Mark node a as visited
    color[a] = 1;
    
    // Visit all nodes reachable from a
    for (uint b = 0; b < bindingAwareSDFG->nrActors(); b++)
    {
        // Node b reachable from a?
        if (abstractDepGraph[a][b])
        {
            // Visited node b before?
            if (color[b] == 1)
            {
                // Found a cycle in the graph containing node b
                c = a;
                d = b;           
                do
                {
                    // All channels from d to c in the SDFG have 
                    // storage dependency
                    for (SDFchannelsIter iter = bindingAwareSDFG->channelsBegin();
                            iter != bindingAwareSDFG->channelsEnd(); iter++)
                    {
                        SDFchannel *ch = *iter;
                        CId srcId = ch->getSrcActor()->getId();
                        CId dstId = ch->getDstActor()->getId();
                        
                        if (dstId == d && srcId == c)
                            dep[ch->getId()] = true;
                    }

                    // Next
                    d = c;
                    c = pi[d];
                } while (d != b);
            }
            else
            {
                // Visit node b
                pi[b] = a;
                dfsVisitDependencies(b, color, pi, abstractDepGraph, dep);
            }
        }
    }

    // Discovered all cycles which actor a is part of. Remove its edges to avoid
    // re-discovering same cycles again.
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++)
    {
        abstractDepGraph[i][a] = false;
        abstractDepGraph[a][i] = false;
    }

    // Mark node a as not visited
    color[a] = 0;
}

/**
 * findStorageDependencies ()
 * The function find all cycles in the abstract dependency graph. Any channel
 * modeling storage space which is part of a cycle is marked to have a storage
 * dependency.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
    ::findStorageDependencies(bool **abstractDepGraph, bool *dep,
        bool *bufferChannels)
{
    int *color, *pi;
    
    // Initialize DFS data structures
    color = new int [bindingAwareSDFG->nrActors()];
    pi = new int [bindingAwareSDFG->nrActors()];
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++)
        color[i] = 0;
        
    // Initialize storage dependencies
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++)
        dep[c] = false;
    
    // DFS from every node in the graph to find all cycles
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++)
    {
        pi[i] = i;
        dfsVisitDependencies(i, color, pi, abstractDepGraph, dep);
    }
    
    // Only channels that have a dependency and that are a buffer channel are
    // true storage dependencies
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++)
        dep[c] = dep[c] && bufferChannels[c];
    
    // Cleanup
    delete [] color;
    delete [] pi;
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
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
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
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
    ::startActorFiring(TimedSDFactor *a)
{
    SDFtime execTime, completionTime, timeTileStartOfSlice, waitingTime;
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
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
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
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
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
SDFtime SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
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

    return step;
}

/**
 * findCausalDependencies ()
 * The function tracks all causal dependencies in the actor firing of actor a.
 * Any causal dependency that is found is added to the abstract dependency
 * graph.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
    ::findCausalDependencies(SDFactor *a, bool **abstractDepGraph)
{
    // Check all input ports for tokens and output ports for space
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        SDFactor *srcActor = c->getSrcActor();
        SDFactor *dstActor = c->getDstActor();
        
        // Actor is destination of the channel?
        if (p->getType() == SDFport::In)
        {
            // Not enough tokens in the previous state?
            if (!CH_TOKENS_PREV(c->getId(), p->getRate()))
            {
                abstractDepGraph[dstActor->getId()][srcActor->getId()] = true;
            }    
        }
    }
}

/**
 * analyzePeriodicPhase ()
 * Analyze the periodic phase of the schedule to find all blocked channels. This
 * is done using the abstract dependency graph.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem
    ::analyzePeriodicPhase(bool *dep, bool *bufferChannels)
{
    State periodicState;
    bool **abstractDepGraph;
    TTime clkStep;
    int repCnt;  

    // Current state is a periodic state
    periodicState = currentState;

    // Abstract dependency graph
    abstractDepGraph = new bool* [bindingAwareSDFG->nrActors()];
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++)
    {
        abstractDepGraph[i] = new bool [bindingAwareSDFG->nrActors()];
        for (uint j = 0; j < bindingAwareSDFG->nrActors(); j++)
            abstractDepGraph[i][j] = false;
    }

    // Start new iteration of the periodic phase
	currentState.glbClk = 0;
    
    // Still need to complete the last firing of the output actor
    // before period really ends
    repCnt = -1;

    // Complete the remaining actor firings
    for (SDFactorsIter iter = bindingAwareSDFG->actorsBegin();
            iter != bindingAwareSDFG->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        
        while (actorReadyToEnd(a))  
        {
            if (a->getId() == outputActor->getId())
            {
                repCnt++;
                if (repCnt == outputActorRepCnt)
                {
                    currentState.glbClk = 0;
                    repCnt = 0;
                }
            }

            endActorFiring(a);
        }
    }

    // Fire the actors
    while (true)
    {
        // Start actor firings
        for (SDFactorsIter iter = bindingAwareSDFG->actorsBegin();
                iter != bindingAwareSDFG->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)(*iter);

            // Ready to fire actor a?
            while (actorReadyToFire(a))
            {
                // Track causal dependencies on firing of actor a
                findCausalDependencies(a, abstractDepGraph);
                
                // Fire actor a
                startActorFiring(a);
            }
        }

        // Clock step
        clkStep = clockStep();

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
                        // Found periodic state
                        if (currentState == periodicState)
                        {
                            // Cycles in the dependency graph indicate storage
                            // dependencies
                            findStorageDependencies(abstractDepGraph, dep,
                                                                bufferChannels);

                            // Cleanup
                            for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++)
                                delete [] abstractDepGraph[i];
                            delete [] abstractDepGraph;

                            // Done
                            return;
                        }
	                    currentState.glbClk = 0;
                        repCnt = 0;  
                    }  
                }

                // End the actor firing
                endActorFiring(a);
            }  
        }
    }
}

/**
 * analyzeDeadlock ()
 * Analyze the deadlock in the schedule.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::analyzeDeadlock(
        bool *dep, bool *bufferChannels)
{
    bool **abstractDepGraph;
    
    // Abstract dependency graph
    abstractDepGraph = new bool* [bindingAwareSDFG->nrActors()];
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++)
    {
        abstractDepGraph[i] = new bool [bindingAwareSDFG->nrActors()];
        for (uint j = 0; j < bindingAwareSDFG->nrActors(); j++)
            abstractDepGraph[i][j] = false;
    }

    // Check number of tokens on every channel in the graph
    for (SDFchannelsIter iter = bindingAwareSDFG->channelsBegin();
            iter != bindingAwareSDFG->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        SDFactor *srcActor = c->getSrcActor();
        SDFactor *dstActor = c->getDstActor();
        uint procDstActor = bindingAwareSDFG->getBindingOfActorToTile(dstActor);
        
        // Destination actor allowed to fire (i.e. not bound to processor or
        // first actor in a schedule?)
        if (procDstActor == ACTOR_NOT_BOUND ||
                SOS_ENTRY(procDstActor)->actor->getId() == dstActor->getId())
        {
            // Insufficient tokens to fire destination actor
            if (!CH_TOKENS(c->getId(), c->getDstPort()->getRate()))
            {
                abstractDepGraph[dstActor->getId()][srcActor->getId()] = true;
            }
        }
    }

    // Cycles in the dependency graph indicate storage dependencies
    findStorageDependencies(abstractDepGraph, dep, bufferChannels);
    
    // Cleanup
    for (uint i = 0; i < bindingAwareSDFG->nrActors(); i++)
        delete [] abstractDepGraph[i];
    delete [] abstractDepGraph;
}

/**  
 * execSDFgraph()  
 * Execute the SDF graph till a deadlock is found or a recurrent state.  
 * The throughput is returned.  
 */  
TDtime SDFstateSpaceBindingAwareBufferAnalysis::TransitionSystem::execSDFgraph(
        const TBufSize *sp, bool *dep, bool *bufferChannels)  
{
    StatesIter recurrentState;
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
        TimedSDFchannel *ch = (TimedSDFchannel*)*iter;

        // Channel models buffer space?
        if (bufferChannels[ch->getId()])
        {
            // Produce as many tokens in the channel as 'sp' indicates
            PRODUCE(ch->getId(), sp[ch->getId()]);
        }
        else if (ch->getInitialTokens() != 0)
        {
            PRODUCE(ch->getId(), ch->getInitialTokens());
        }
                    
        // Remove as many tokens from the channel as needed to store
        // the initial tokens of the channel it models
        if (ch->modelsStorageSpace())
        {
            TimedSDFchannel *c = (TimedSDFchannel*)ch->getStorageSpaceChannel();
            
            // Related channel contains initial tokens?
            if (c->getInitialTokens() > 0)
            {
                // Channel ch models either memory buffer of source connection
                // buffer. Channels modelling destination buffer do not contain
                // initial tokens.
                if (c->getSrcActor()->getId() == ch->getDstActor()->getId())
                {
                    // Sufficient tokens in channel ch to store all initial
                    // tokens in channel c?
                    if (CH_TOKENS(ch->getId(), c->getInitialTokens()))
                    {
                        CONSUME(ch->getId(), c->getInitialTokens());
                    }
                    else
                    {
                        if (bufferChannels[ch->getId()])
                            dep[ch->getId()] = true;
                        return 0;
                    }
                }
            }
        }
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
                            // Find storage dependencies in periodic phase
                            analyzePeriodicPhase(dep, bufferChannels);
                            
                            // Done
                            return computeThroughput(recurrentState);
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
            // Find storage dependencies in deadlock
            analyzeDeadlock(dep, bufferChannels);
            
            // Done
            return 0;
        }
    }
    
    return 0;
}  

/*******************************************************************************
 * Distributions
 ******************************************************************************/

/**
 * newStorageDistribution ()
 * Allocate memory for a new storage distribution.
 */
StorageDistribution *SDFstateSpaceBindingAwareBufferAnalysis
    ::newStorageDistribution()
{
    StorageDistribution *d;
    
    d = new StorageDistribution;
    d->sp = new TBufSize [bindingAwareSDFG->nrChannels()];
    d->dep = new bool [bindingAwareSDFG->nrChannels()];
    
    return d;
}

/**
 * deleteStorageDistribution ()
 * Deallocate memory for a storage distribution.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::deleteStorageDistribution(
        StorageDistribution *d)
{
    delete [] d->sp;
    delete [] d->dep;
    delete d;
}

/**
 * execStorageDistribution ()
 * Compute throughput and storage dependencies of the given storage
 * distribution.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::execStorageDistribution(
        StorageDistribution *d, bool *bufferChannels)
{
    // Initialize blocking channels
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++)
        d->dep[c] = false;

    // Execute the SDF graph to find its output interval
    d->thr = transitionSystem->execSDFgraph(d->sp, d->dep, bufferChannels);

    //cerr << d->sz << " " << d->thr << endl;
    //for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++)
    //    cerr << d->sp[c] << " " << d->dep[c] << endl;
    //cerr << endl;
}

/**
 * minimizeMinStorageDistributions ()
 * The function removes all storage distributions within the supplied
 * set of storage distributions which are non-minimal. All storage distributions
 * within the set should have the same size.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::minimizeStorageDistributionsSet(
        StorageDistributionSet *ds)
{
    StorageDistribution *d, *t;
    
    // Maximal throughput with current distribution size equal to previous
    // throughput with previous (smaller) distribution size
    if (ds->prev != NULL && ds->prev->thr == ds->thr)
    {
        // No minimal storage distributions exist in this list
        // Iterate over the list of storage distributions    
        d = ds->distributions;
        while (d != NULL)
        {
            // Temporary reference to next element in list
            t = d->next;

            // Cleanup d
            deleteStorageDistribution(d);

            // Next
            d = t;
        }
        
        // No distributions left
        ds->distributions = NULL;
    }
    else
    {
        // Iterate over the list of storage distributions    
        d = ds->distributions;
        while (d != NULL)
        {
            // Throughput of distribution smaller then maximum throughput with
            // same distribution size?
            if (d->thr < ds->thr)
            {
                // Remove d from linked list
                if (d->prev != NULL)
                    d->prev->next = d->next;
                else
                    ds->distributions = d->next;
                if (d->next != NULL)
                    d->next->prev = d->prev;

                // Temporary reference to next element in list
                t = d->next;

                // Cleanup d
                deleteStorageDistribution(d);

                // Next
                d = t;
            }
            else
            {
                // Next
                d = d->next;
            }
        }
    }
}

/**
 * addStorageDistributionToChecklist ()
 * The function add the storage distribution 'd' to the list of
 * storage distributions which must still be checked. This list is
 * ordered wrt to the size of the storage distribution. A storage
 * distribution is only added if it is not already contained in the
 * list. When the distribution is added to the list, the function returns
 * 'true', else the function returns 'false'.
 */
bool SDFstateSpaceBindingAwareBufferAnalysis::addStorageDistributionToChecklist(
        StorageDistribution *d)
{
    StorageDistributionSet *ds, *dsNew;
    StorageDistribution *di;
    bool equalDistr;
    
    // First distribution ever added?
    if (minStorageDistributions == NULL)
    {
        // Create new set of storage distributions
        dsNew = new StorageDistributionSet;
        dsNew->sz = d->sz;
        dsNew->thr = 0;
        dsNew->distributions = d;
        dsNew->next = NULL;
        dsNew->prev = NULL;
        
        // Set is first in list to be checked
        minStorageDistributions = dsNew;
        
        // Done
        return true;
    }
    
    // Find set of distributions with the same size
    ds = minStorageDistributions;
    while (ds->next != NULL && ds->next->sz <= d->sz)
    {
        ds = ds->next;
    }
    
    ASSERT(ds != NULL, "checkDistributions list cannnot be empty");
    
    // Set of storage distribution with same size as d exists?
    if (ds->sz == d->sz)
    {

        // Check that distribution d does not exist in the set
        di = ds->distributions;
        while (di != NULL)
        {
            equalDistr = true;

            // All channels have same storage space allocation?
            for (uint c = 0; equalDistr 
                                && c < bindingAwareSDFG->nrChannels(); c++)
            {
                if (di->sp[c] != d->sp[c])
                    equalDistr = false;
            }

            // Found equal distribution
            if (equalDistr)
                return false;

            // Next
            di = di->next;
        }

        // Distribution 'd' not yet in the set, so let's add it
        if (ds->distributions != NULL)
            ds->distributions->prev = d;
        d->next = ds->distributions;
        ds->distributions = d;
    }
    else if (ds->next == NULL)
    {
        // No set of distribution in the list with same or larger size?	
        
        // Create new set of storage distributions
        dsNew = new StorageDistributionSet;
        dsNew->sz = d->sz;
        dsNew->thr = 0;
        dsNew->distributions = d;
        dsNew->next = NULL;
        dsNew->prev = ds;
        
        // Add set to the list of sets
        ds->next = dsNew;
    }    
    else
    {
        // In this case 'ds->next' has a size larger then 'd' and 'ds' has
        // a size smaller then 'd'. Insert a new set of storage
        // distributions between 'ds' and 'ds->next' containing storage
        // distribution 'd'.

        // Create new set of storage distributions
        dsNew = new StorageDistributionSet;
        dsNew->sz = d->sz;
        dsNew->thr = 0;
        dsNew->distributions = d;
        dsNew->next = ds->next;
        dsNew->prev = ds;

        // Add set to the list of sets
        ds->next->prev = dsNew;
        ds->next = dsNew;
    }
    
    // Done
    return true;
}

/**
 * exploreStorageDistribution ()
 * The function computes the throughput of the storage distribution
 * 'd' and adds new storage distributions to the list of distributions
 * which must be checked based on the storage dependencies found in d.
 * The function also updates the maximal throughput of the set of
 * storage distributions when needed.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::exploreStorageDistribution(
        StorageDistributionSet *ds,  StorageDistribution *d, 
        bool *bufferChannels, bool useBoundsOnBufferChannels)
{
    StorageDistribution *dNew;
    
    // Compute throughput and storage dependencies
    execStorageDistribution(d, bufferChannels);
    
    // Throughput of d larger then current maximum of the set
    if (d->thr > ds->thr)
        ds->thr = d->thr;
    
    // Create new storage distributions for every channel which
    // has a storage dependency in d
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++)
    {
        // Channel c has storage dependency?
        if (d->dep[c])
        {
            // Do not enlarge the channel if its a self-edge
            if (bindingAwareSDFG->getChannel(c)->getSrcActor()->getId() 
                    == bindingAwareSDFG->getChannel(c)->getDstActor()->getId())
                continue;

            // Do not enlarge channel above its current bound             
            if (useBoundsOnBufferChannels 
                    && d->sp[c] 
                        >= bindingAwareSDFG->getChannel(c)->getInitialTokens())
            {
                continue;
            }
                    
            // Create new storage distribution with channel c enlarged
            dNew = newStorageDistribution();
            dNew->sz = d->sz + minSzStep[c];
            dNew->thr = 0;
            for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++)
            {
                dNew->sp[i] = d->sp[i];
                if (i == c)
                    dNew->sp[i] += minSzStep[c];
            }
            dNew->next = NULL;
            dNew->prev = NULL;
            
            // Add storage distribution to set of distributions to be checked
            if (!addStorageDistributionToChecklist(dNew))
            {
                // Distribution already in check list
                deleteStorageDistribution(dNew);
            }
        }
    }
}

/**
 * exploreStorageDistributionSet ()
 * Explore all distributions within the set and remove all non-minimal
 * distributions from it.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::exploreStorageDistributionSet(
        StorageDistributionSet *ds, bool *bufferChannels,
        bool useBoundsOnBufferChannels)
{
    StorageDistribution *d;

    // Explore all storage distributions contained in the set    
    d = ds->distributions;
    while (d != NULL)
    {
        // Explore distribution d
        exploreStorageDistribution(ds, d, bufferChannels,
                                        useBoundsOnBufferChannels);
        
        // Next
        d = d->next;
    }
    
    // Remove all non-minimal storage distributions from the set
    minimizeStorageDistributionsSet(ds);
}

/**
 * findMinimalStorageDistributions ()
 * Explore the throughput/storage-size trade-off space till either
 * all minimal storage distributions are found or the throughput bound is
 * reached.
 */
void SDFstateSpaceBindingAwareBufferAnalysis::findMinimalStorageDistributions(
        const double thrBound, bool *bufferChannels,
        bool useBoundsOnBufferChannels)
{
    StorageDistribution *d, *t;
    StorageDistributionSet *ds, *dt;
    
    // Construct storage distribution with lower bound storage space
    d = newStorageDistribution();
    d->thr = 0;
    d->sz = lbDistributionSz;
    for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++)
        d->sp[c] = minSz[c];
    d->next = NULL;
    d->prev = NULL;
    
    // Add distribution to set of distributions which must be checked
    addStorageDistributionToChecklist(d);
    
    // Check sets of storage distributions till no distributions left to check,
    // or throughput bound exceeded, or maximal throughput reached
    ds = minStorageDistributions;
    while (ds != NULL)
    {
        // Explore all distributions with size 'ds->sz'
        exploreStorageDistributionSet(ds, bufferChannels,
                                            useBoundsOnBufferChannels);

        // Reached maximum throughput or exceed thrBound
        if (ds->thr >= thrBound || ds->thr == maxThroughput)
            break;
        
        // Temporary pointer to set
        dt = ds;

        // Next
        ds = ds->next;

        // No distributions left with the given distribution size?
        if (dt->distributions == NULL)
        {
            // Remove distr from linked list
            if (dt->prev != NULL)
                dt->prev->next = dt->next;
            else
                minStorageDistributions = dt->next;
            if (dt->next != NULL)
                dt->next->prev = dt->prev;

            // Cleanup dt
            delete dt;
        }
    }
    
    // Unexplored distributions left?
    if (ds != NULL && ds->next != NULL)
    {
        // Pointer to first set which must be removed
        ds = ds->next;
        
        // Mark previous set as last set in list of minimal storage distr
        ds->prev->next = NULL;
        
        // Remove all unexplored distributions (and sets)
        while (ds != NULL)
        {
            // Remove all distributions within the set ds
            d = ds->distributions;
            while (d != NULL)
            {
                t = d->next;
                deleteStorageDistribution(d);
                d = t;
            }

            // Temporary pointer to set
            dt = ds;
            
            // Next
            ds = ds->next;

            // Cleanup dt
            delete dt;
        }
    }
    
    // Lower bound on storage space (which is used in the beginning) is not a
    // minimal storage distribution if it deadlocks. The distribution <0,...,0>
    // is the actual minimal storage distribution for this throughput
    if (minStorageDistributions->thr == 0)
    {
        minStorageDistributions->sz = 0;
        minStorageDistributions->distributions->sz = 0;
        for (uint c = 0; c < bindingAwareSDFG->nrChannels(); c++)
            minStorageDistributions->distributions->sp[c] = 0;
    }
}

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
StorageDistributionSet *SDFstateSpaceBindingAwareBufferAnalysis::analyze(
        BindingAwareSDFG *bg, bool *bufferChannels, 
        bool useBoundsOnBufferChannels, const double thrBound)
{
    // Binding-aware graph
    bindingAwareSDFG = bg;

    // Start with an empty set of storage distributions
    minStorageDistributions = NULL;

    // Initialize bounds on the search space
    initBoundsSearchSpace(bindingAwareSDFG, bufferChannels);

    // Create a transition system
    transitionSystem = new TransitionSystem(bindingAwareSDFG);

    // Search the space
    findMinimalStorageDistributions(thrBound, bufferChannels,
                                        useBoundsOnBufferChannels);

    // Cleanup
    delete [] minSz;
    delete [] minSzStep;
    delete transitionSystem;
    
    return minStorageDistributions;
}
