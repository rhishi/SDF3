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
 * $Id: buffer.cc,v 1.1 2008/03/06 10:49:42 sander Exp $
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

#include "buffer.h"
#include "../../base/algo/repetition_vector.h"
#include "../throughput/throughput.h"

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

/******************************************************************************
 * Bounds on the search space
 *****************************************************************************/

/**
 * initBoundsSearchSpace ()
 * Compute bounds on the trade-off space that must be explored.
 */
void SDFstateSpaceBufferAnalysis::initBoundsSearchSpace(TimedSDFgraph *g)
{
    initMinimalChannelSzStep(g);
    initMinimalChannelSz(g);
    initLbDistributionSz(g);
    initMaxThroughput(g);
}

/**
 * initMinimalChannelSzStep ()
 * Compute lower bound on the step size of channels
 */
void SDFstateSpaceBufferAnalysis::initMinimalChannelSzStep(TimedSDFgraph *g)
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
void SDFstateSpaceBufferAnalysis::initMinimalChannelSz(TimedSDFgraph *g)
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
        
        lb = p+c-gcd(p,c)+t%gcd(p,c);
        lb = (lb > t ? lb : t);
        
        minSz[ch->getId()] = lb;
        
        // Lower-bound of a self-edge is rate at which data is produced and
        // consumed and the number of initial tokens present
        if (ch->getSrcActor()->getId() == ch->getDstActor()->getId())
            minSz[ch->getId()] = p + (c > t ? c : t);
    }
}

/**
 * initLbDistributionSz ()
 * Compute lower bound on the buffer size needed for positive throughput
 */
void SDFstateSpaceBufferAnalysis::initLbDistributionSz(TimedSDFgraph *g)
{
    lbDistributionSz = 0;
    
    for (uint c = 0; c < g->nrChannels(); c++)
        lbDistributionSz += minSz[c];
}

/**
 * initMaxThroughput ()
 * Compute the maximal throughput that can be achieved by the graph.
 */
void SDFstateSpaceBufferAnalysis::initMaxThroughput(TimedSDFgraph *g)
{
    SDFstateSpaceThroughputAnalysis thrAnalysisAlgo;
    
    maxThroughput = thrAnalysisAlgo.analyze(g);
}

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceBufferAnalysis::TransitionSystem::State::print(
        ostream &out)
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

    for (uint i = 0; i < sp.size(); i++)
    {
        out << "sp[" << i << "] = " << ch[i] << endl;
    }

    out << "glbClk = " << glbClk << endl;
}

/**
 * clearState ()
 * The function sets the state to zero.
 */
void SDFstateSpaceBufferAnalysis::TransitionSystem::State::clear()
{
    for (uint i = 0; i < actClk.size(); i++)
    {
        actClk[i].clear();
    }

    for (uint i = 0; i < ch.size(); i++)
    {
        ch[i] = 0;
        sp[i] = 0;
    }
    
    glbClk = 0;
}

/**
 * operator== ()
 * The function compares to states and returns true if they are equal.
 */
bool SDFstateSpaceBufferAnalysis::TransitionSystem::State::operator==(
    const State &s)
{
    if (glbClk != s.glbClk)
        return false;
    
    for (uint i = 0; i < ch.size(); i++)
    {
        if (ch[i] != s.ch[i] || sp[i] != s.sp[i])
            return false;
    }
    
    for (uint i = 0; i < actClk.size(); i++)
    {
        if (actClk[i] != s.actClk[i])
            return false;
    }
    
    return true;
}

/******************************************************************************
 * Transition system
 *****************************************************************************/

/**
 * initOutputActor ()
 * The function selects an actor to be used as output actor in the
 * state transition system.
 */
void SDFstateSpaceBufferAnalysis::TransitionSystem::initOutputActor()
{
    RepetitionVector repVec;
    int min = INT_MAX;
    SDFactor *a = NULL;

    // Compute repetition vector
    repVec = computeRepetitionVector(g);
    
    // Select actor with lowest entry in repetition vector as output actor
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
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
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the state s is not stored. The function
 * returns false. The function always sets the pos variable to the position
 * where the state s is in the list.
 */
bool SDFstateSpaceBufferAnalysis::TransitionSystem::storeState(State &s,
        StatesIter &pos)
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
TDtime SDFstateSpaceBufferAnalysis::TransitionSystem::computeThroughput(
        const StatesIter cycleIter)  
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
void SDFstateSpaceBufferAnalysis::TransitionSystem::dfsVisitDependencies(uint a,
        int *color, int *pi, bool **abstractDepGraph, bool *dep)
{
    uint c, d;

    // Mark node a as visited
    color[a] = 1;
    
    // Visit all nodes reachable from a
    for (uint b = 0; b < g->nrActors(); b++)
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
                    for (SDFchannelsIter iter = g->channelsBegin();
                            iter != g->channelsEnd(); iter++)
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
    for (uint i = 0; i < g->nrActors(); i++)
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
void SDFstateSpaceBufferAnalysis::TransitionSystem::findStorageDependencies(
        bool **abstractDepGraph, bool *dep)
{
    int *color, *pi;
    
    // Initialize DFS data structures
    color = new int [g->nrActors()];
    pi = new int [g->nrActors()];
    for (uint i = 0; i < g->nrActors(); i++)
        color[i] = 0;
        
    // Initialize storage dependencies
    for (uint c = 0; c < g->nrChannels(); c++)
        dep[c] = false;
    
    // DFS from every node in the graph to find all cycles
    for (uint i = 0; i < g->nrActors(); i++)
    {
        pi[i] = i;
        dfsVisitDependencies(i, color, pi, abstractDepGraph, dep);
    }
    
    // Cleanup
    delete [] color;
    delete [] pi;
}

/******************************************************************************
 * SDF
 *****************************************************************************/

#define CH(c)               currentState.ch[c]
#define SP(c)               currentState.sp[c]  
#define CH_TOKENS(c,n)      (CH(c) >= n)  
#define CH_SPACE(c,n)       (SP(c) >= n)
#define CONSUME(c,n)        CH(c) = CH(c) - n;
#define PRODUCE(c,n)        CH(c) = CH(c) + n;
#define CONSUME_SP(c,n)     SP(c) = SP(c) - n;
#define PRODUCE_SP(c,n)     SP(c) = SP(c) + n;

#define CH_TOKENS_PREV(c,n) (previousState.ch[c] >= n)  
#define CH_SPACE_PREV(c,n)  (previousState.sp[c] >= n)

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceBufferAnalysis::TransitionSystem::actorReadyToFire(
        SDFactor *a)
{
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
        else
        {
            if (!CH_SPACE(c->getId(), p->getRate()))
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
void SDFstateSpaceBufferAnalysis::TransitionSystem::startActorFiring(
        TimedSDFactor *a)
{
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
        else
        {
            CONSUME_SP(c->getId(), p->getRate());
        }
    }

    // Add actor firing to the list of active firings of this actor
    currentState.actClk[a->getId()].push_back(a->getExecutionTime());
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceBufferAnalysis::TransitionSystem::actorReadyToEnd(SDFactor *a)
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
void SDFstateSpaceBufferAnalysis::TransitionSystem::endActorFiring(SDFactor *a)
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
        else
        {
            PRODUCE_SP(c->getId(), p->getRate());
        }
    }

    // Remove the firing from the list of active actor firings
    currentState.actClk[a->getId()].pop_front();
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
SDFtime SDFstateSpaceBufferAnalysis::TransitionSystem::clockStep()
{
    SDFtime step = UINT_MAX;
    
    // Find maximal time progress
    for (uint a = 0; a < g->nrActors(); a++)
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
    for (uint a = 0; a < g->nrActors(); a++)
    {
        for (list<SDFtime>::iterator iter = currentState.actClk[a].begin();
                    iter != currentState.actClk[a].end(); iter++)
        {
            SDFtime &actFiringTime = *iter;
            
            // Lower remaining execution time of the actor firing
            actFiringTime -= step;
        }
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
void SDFstateSpaceBufferAnalysis::TransitionSystem::findCausalDependencies(
        SDFactor *a, bool **abstractDepGraph)
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
        else
        {
            // Not enough space in the previous state?
            if (!CH_SPACE_PREV(c->getId(), p->getRate()))
            {
                abstractDepGraph[srcActor->getId()][dstActor->getId()] = true;
            }
        }
        
    }
}

/**
 * analyzePeriodicPhase ()
 * Analyze the periodic phase of the schedule to find all blocked channels. This
 * is done using the abstract dependency graph.
 */
void SDFstateSpaceBufferAnalysis::TransitionSystem::analyzePeriodicPhase(
        const TBufSize *sp, bool *dep)
{
    bool **abstractDepGraph;
    State periodicState(g->nrActors(), g->nrChannels());
    TTime clkStep;
    int repCnt;  

    // Current state is a periodic state
    periodicState = currentState;

    // Abstract dependency graph
    abstractDepGraph = new bool* [g->nrActors()];
    for (uint i = 0; i < g->nrActors(); i++)
    {
        abstractDepGraph[i] = new bool [g->nrActors()];
        for (uint j = 0; j < g->nrActors(); j++)
            abstractDepGraph[i][j] = false;
    }

    // Start new iteration of the periodic phase
	currentState.glbClk = 0;
    
    // Still need to complete the last firing of the output actor
    // before period really ends
    repCnt = -1;

    // Complete the remaining actor firings
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
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
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
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
        for (uint i = 0; i < g->nrChannels(); i++)
        {
            previousState.ch[i] = currentState.ch[i];
            previousState.sp[i] = currentState.sp[i];
        }

        // Finish actor firings  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
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
                            findStorageDependencies(abstractDepGraph, dep);

                            // Cleanup
                            for (uint i = 0; i < g->nrActors(); i++)
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
void SDFstateSpaceBufferAnalysis::TransitionSystem::analyzeDeadlock(
        const TBufSize *sp, bool *dep)
{
    bool **abstractDepGraph;
    
    // Abstract dependency graph
    abstractDepGraph = new bool* [g->nrActors()];
    for (uint i = 0; i < g->nrActors(); i++)
    {
        abstractDepGraph[i] = new bool [g->nrActors()];
        for (uint j = 0; j < g->nrActors(); j++)
            abstractDepGraph[i][j] = false;
    }

    // Check number of tokens on every channel in the graph
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        SDFactor *srcActor = c->getSrcActor();
        SDFactor *dstActor = c->getDstActor();
        
        // Insufficient tokens to fire destination actor
        if (!CH_TOKENS(c->getId(), c->getDstPort()->getRate()))
        {
            abstractDepGraph[dstActor->getId()][srcActor->getId()] = true;
        }

        // Insufficient space to fire source actor
        if (!CH_SPACE(c->getId(), c->getSrcPort()->getRate()))
        {
            abstractDepGraph[srcActor->getId()][dstActor->getId()] = true;
        }
    }

    // Cycles in the dependency graph indicate storage dependencies
    findStorageDependencies(abstractDepGraph, dep);
    
    // Cleanup
    for (uint i = 0; i < g->nrActors(); i++)
        delete [] abstractDepGraph[i];
    delete [] abstractDepGraph;
}

/**  
 * execSDFgraph()  
 * Execute the SDF graph till a deadlock is found or a recurrent state.  
 * The throughput is returned.  
 */  
TDtime SDFstateSpaceBufferAnalysis::TransitionSystem::execSDFgraph(
        const TBufSize *sp, bool *dep)  
{
    StatesIter recurrentState;
    TTime clkStep;
    int repCnt = 0;  

    // Clear the list of stored states
    clearStoredStates();

    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();  
    previousState.init(g->nrActors(), g->nrChannels());
    previousState.clear();  

    // Initial tokens and space
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;

        // Not enough space for initial tokens?
        if (sp[c->getId()] < c->getInitialTokens())
        {
            dep[c->getId()] = true;
            return 0;
        }

        CH(c->getId()) = c->getInitialTokens();
        SP(c->getId()) = sp[c->getId()] - c->getInitialTokens();
    }

    // Fire the actors  
    while (true)  
    {
        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++)
        {
            previousState.ch[i] = currentState.ch[i];
            previousState.sp[i] = currentState.sp[i];
        }

        // Finish actor firings  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
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
                            analyzePeriodicPhase(sp, dep);

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
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
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
            // Find cause of deadlock
            analyzeDeadlock(sp, dep);
            return 0;
        }
    }
    
    return 0;
}  

/*******************************************************************************
 * Storage distributions
 ******************************************************************************/

/**
 * newStorageDistribution ()
 * Allocate memory for a new storage distribution.
 */
StorageDistribution *SDFstateSpaceBufferAnalysis::newStorageDistribution()
{
    StorageDistribution *d;
    
    d = new StorageDistribution;
    d->sp = new TBufSize [g->nrChannels()];
    d->dep = new bool [g->nrChannels()];
    
    return d;
}

/**
 * deleteStorageDistribution ()
 * Deallocate memory for a storage distribution.
 */
void SDFstateSpaceBufferAnalysis::deleteStorageDistribution(
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
void SDFstateSpaceBufferAnalysis::execStorageDistribution(
        StorageDistribution *d)
{
    // Initialize blocking channels
    for (uint c = 0; c < g->nrChannels(); c++)
        d->dep[c] = false;

    // Execute the SDF graph to find its output interval
    d->thr = transitionSystem->execSDFgraph(d->sp, d->dep);

    //cerr << d->sz << " " << d->thr << endl;
    //for (uint c = 0; c < g->nrChannels(); c++)
    //    cerr << d->sp[c] << " " << d->dep[c] << endl;
    //cerr << endl;
}

/**
 * minimizeMinStorageDistributions ()
 * The function removes all storage distributions within the supplied
 * set of storage distributions which are non-minimal. All storage distributions
 * within the set should have the same size.
 */
void SDFstateSpaceBufferAnalysis::minimizeStorageDistributionsSet(
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
bool SDFstateSpaceBufferAnalysis::addStorageDistributionToChecklist(
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
            for (uint c = 0; equalDistr && c < g->nrChannels(); c++)
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
void SDFstateSpaceBufferAnalysis::exploreStorageDistribution(
        StorageDistributionSet *ds, StorageDistribution *d)
{
    StorageDistribution *dNew;
    
    // Compute throughput and storage dependencies
    execStorageDistribution(d);
    
    // Throughput of d larger then current maximum of the set
    if (d->thr > ds->thr)
        ds->thr = d->thr;
    
    // Create new storage distributions for every channel which
    // has a storage dependency in d
    for (uint c = 0; c < g->nrChannels(); c++)
    {
        // Channel c has storage dependency?
        if (d->dep[c])
        {
            // Do not enlarge the channel if its a self-edge
            if (g->getChannel(c)->getSrcActor()->getId() 
                    == g->getChannel(c)->getDstActor()->getId())
                continue;
                
            // Create new storage distribution with channel c enlarged
            dNew = newStorageDistribution();
            dNew->sz = d->sz + minSzStep[c];
            dNew->thr = 0;
            for (uint i = 0; i < g->nrChannels(); i++)
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
void SDFstateSpaceBufferAnalysis::exploreStorageDistributionSet(
        StorageDistributionSet *ds)
{
    StorageDistribution *d;

    // Explore all storage distributions contained in the set    
    d = ds->distributions;
    while (d != NULL)
    {
        // Explore distribution d
        exploreStorageDistribution(ds, d);
        
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
void SDFstateSpaceBufferAnalysis::findMinimalStorageDistributions(
        const double thrBound)
{
    StorageDistribution *d, *t;
    StorageDistributionSet *ds, *dt;
    
    // Construct storage distribution with lower bound storage space
    d = newStorageDistribution();
    d->thr = 0;
    d->sz = lbDistributionSz;
    for (uint c = 0; c < g->nrChannels(); c++)
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
        exploreStorageDistributionSet(ds);

        // Reached maximum throughput or exceed thrBound
        if (ds->thr >= thrBound || ds->thr == maxThroughput)
        {
            // thrBound equal to zero is a special case
            if (thrBound == 0)
            {
                // Only stop when throughput above zero
                if (ds->thr > 0)
                    break;
            }
            else
            {
                break;
            }
        }
        
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
        for (uint c = 0; c < g->nrChannels(); c++)
            minStorageDistributions->distributions->sp[c] = 0;
    }
}

/**
 * analyze ()
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency). The search ends as soon as the throughput bound (thrBound)
 * is reached. To find the complete pareto-space, the throughput bound should
 * be set to DOUBLE_MAX.
 */
StorageDistributionSet *SDFstateSpaceBufferAnalysis::analyze(
        TimedSDFgraph *gr, const double thrBound)
{
    // SDF graph
    g = gr;

    // Start with an empty set of storage distributions
    minStorageDistributions = NULL;

    // Initialize bounds on the search space
    initBoundsSearchSpace(g);

    // Create a transition system
    transitionSystem = new TransitionSystem(g);

    // Search the space
    findMinimalStorageDistributions(thrBound);

    // Cleanup
    delete [] minSz;
    delete [] minSzStep;
    delete transitionSystem;
    
    return minStorageDistributions;
}

/**
 * initSearch ()
 * Initialize the storage distribution search algorithm.
 */
void SDFstateSpaceBufferAnalysis::initSearch(TimedSDFgraph *gr)
{
    StorageDistribution *d;

    // SDF graph
    g = gr;

    // Start with an empty set of storage distributions
    minStorageDistributions = NULL;

    // Initialize bounds on the search space
    initBoundsSearchSpace(g);

    // Create a transition system
    transitionSystem = new TransitionSystem(g);
    
    // Construct storage distribution with lower bound storage space
    d = newStorageDistribution();
    d->thr = 0;
    d->sz = lbDistributionSz;
    for (uint c = 0; c < g->nrChannels(); c++)
        d->sp[c] = minSz[c];
    d->next = NULL;
    d->prev = NULL;
    
    // Add distribution to set of distributions which must be checked
    addStorageDistributionToChecklist(d);
    
    // No storage distribution set explored so far
    lastExploredStorageDistributionSet = NULL;
}

/**
 * findNextStorageDistributionSet ()
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency). The search ends as soon as a new pareto point has been
 * found in the space or when the full space has been explored. The function
 * returns the newly discovered storage distribution set or NULL when no new set
 * was found.
 */
StorageDistributionSet *
SDFstateSpaceBufferAnalysis::findNextStorageDistributionSet()
{
    StorageDistribution *d, *t;
    StorageDistributionSet *dp, *ds, *dt;

    // Last explored distribution set
    dp = lastExploredStorageDistributionSet;

    // Last explored set reaches already maximal throughput?
    if (dp != NULL && dp->thr == maxThroughput)
        return NULL;

    // Next set to be explored
    if (dp == NULL)
        ds = minStorageDistributions;
    else
        ds = dp->next;

    // Check sets of storage distributions till no distributions left to check,
    // or next pareto point is found
    while (ds != NULL)
    {
        // Explore all distributions with size 'ds->sz'
        exploreStorageDistributionSet(ds);

        // Distribution set ds has larger throughput then previous pareto point?
        if ((dp == NULL && ds->thr > 0)
                || (dp != NULL && ds->thr > dp->thr))
        {
            break;
        }
        
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
    
    // Unexplored distributions left, but maximal throughput reached?
    if (ds != NULL && ds->next != NULL && ds->thr == maxThroughput)
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
        for (uint c = 0; c < g->nrChannels(); c++)
            minStorageDistributions->distributions->sp[c] = 0;
    }
    
    // Current set becomes last explored set
    lastExploredStorageDistributionSet = ds;
    
    return ds;
}
