/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   minimal.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   September 14, 2006
 *
 *  Function        :   Compute minimal latency of SDFG for all possible
 *                      executions.
 *
 *  History         :
 *      14-09-06    :   Initial version.
 *      12-02-08    :   Complete re-write in which code is restructed into
 *                      a C++ class.
 *
 * $Id: minimal.cc,v 1.1 2008/03/06 10:49:43 sander Exp $
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

#include "minimal.h"
#include "../../base/algo/repetition_vector.h"
#include "../../base/algo/components.h"

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::State::print(
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

    out << "glbClk = " << glbClk << endl;
}

/**
 * clearState ()
 * The function sets the state to zero.
 */
void SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::State::clear()
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
}

/**
 * operator== ()
 * The function compares to states and returns true if they are equal.
 */
bool SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::State::operator==(
    const State &s)
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
void SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::initOutputActor()
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
    outputActor = a->getId();
    outputActorRepCnt = repVec[outputActor];
}

/**
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the state s is not stored. The function
 * returns false. The function always sets the pos variable to the position
 * where the state s is in the list.
 */
bool SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::storeState(
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
TDtime SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem
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
 * SDF
 *****************************************************************************/

#define CH(c)               currentState.ch[c]
#define CH_TOKENS(c,n)      (CH(c) >= n)  
#define CONSUME(c,n)        CH(c) = CH(c) - n;
#define PRODUCE(c,n)        CH(c) = CH(c) + n;

#define CH_TOKENS_PREV(c,n) (previousState.ch[c] >= n)  

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::actorReadyToFire(
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
    }

    return true;
}

/**
 * startActorFiring ()
 * Start the actor firing. Remove tokens from all input channels and add the
 * actor firing to the list of active actor firings and advance sequence
 * position.
 */
void SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::startActorFiring(
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
    }

    // Add actor firing to the list of active firings of this actor
    currentState.actClk[a->getId()].push_back(a->getExecutionTime());
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::actorReadyToEnd(
        SDFactor *a)
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
void SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::endActorFiring(
        SDFactor *a)
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
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
SDFtime SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::clockStep()
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
 * execSDFgraph()  
 * Execute the SDF graph till a deadlock is found. The total elapsed time till
 * the deadlock occurs is returned.
 */  
TDtime SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::execSDFgraph(
        bool startFromInitialState)  
{
    SDFtime clkStep, globalTime = 0;

    if (startFromInitialState)
    {
        // Clear the stored states
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

            CH(c->getId()) = c->getInitialTokens();
        }
    }

    // Fire the actors  
    while (true)  
    {
        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++)
        {
            previousState.ch[i] = currentState.ch[i];
        }

        // Finish actor firings  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            
            while (actorReadyToEnd(a))  
            {
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
            return globalTime;
        }
        
        // Advance the global time (this is the total time since the beginning
        // of everything)
        globalTime += clkStep;
    }
    
    return 0;
}  

/**  
 * execSDFgraphUsingDemandList ()  
 * Execute the SDF graph in a self-timed manner. Only actors in the demand list
 * may be executed (upto the number of times specified). The function returns
 * the amount of time needed to execute all actors from the demand list. 
 * It starts from the current state.
 */  
SDFtime SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem
        ::execSDFgraphUsingDemandList(vector<uint> &demandList)  
{
    SDFtime clkStep, globalTime = 0;
    bool noActorFiringsNeeded;
    
    // Fire the actors  
    while (true)  
    {
        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++)
        {
            previousState.ch[i] = currentState.ch[i];
        }

        // Finish actor firings  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            
            while (actorReadyToEnd(a))  
            {
                // End the actor firing
                endActorFiring(a);
            }  
        }

        // Start actor firings  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)(*iter);
        
            // Ready to fire actor a and allowed to fire?
            while (actorReadyToFire(a) && demandList[a->getId()] != 0)
            {
                // Fire actor a
                startActorFiring(a);

                // Decrease number of remaining actor firings in demand list
                demandList[a->getId()] = demandList[a->getId()] - 1;
            }
        }

        // No actor firings left in demand list?
        noActorFiringsNeeded = true;
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd() && noActorFiringsNeeded; iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)*iter;
        
            if (demandList[a->getId()] != 0)
                noActorFiringsNeeded = false;
        }
        if (noActorFiringsNeeded)
            return globalTime;

        // Clock step
        clkStep = clockStep();
        
        // Deadlocked?
        if (clkStep == UINT_MAX)
        {
            return UINT_MAX;
        }

        // Advance the global time (this is the total time since the beginning
        // of everything)
        globalTime += clkStep;
    }
    
    return 0;
}  

/**
 * computeDemandList ()
 * The function computes the minimal number of firings required of each actor to
 * enable the required number of firings of the destination actor connected to
 * the channel ch.
 */
void SDFstateSpaceMinimalLatencyAnalysis::TransitionSystem::computeDemandList(
        const SDFchannel *ch, vector<uint> &demandList)
{
    SDFactor *srcActor, *dstActor;
    SDFport *srcPort, *dstPort;
    uint nrFiringsNeeded;
    int nrTokensNeeded;
    
    // Source and destination of channel
    srcActor = ch->getSrcActor();
    srcPort = ch->getSrcPort();
    dstActor = ch->getDstActor();
    dstPort = ch->getDstPort();
    
    // Number of tokens which need to be produced by source actor 
    // for destination actor
    nrTokensNeeded = dstPort->getRate() * demandList[dstActor->getId()];
    nrTokensNeeded = nrTokensNeeded - CH(ch->getId());
    
    // No tokens needed
    if (nrTokensNeeded < 0)
        return;
    
    // Compute number of firings needed of source actor
    nrFiringsNeeded = nrTokensNeeded / srcPort->getRate();
    if (nrTokensNeeded % srcPort->getRate() != 0)
        nrFiringsNeeded++;
    
    // Number of firings needed larger then what is currently known for source
    // actor?
    if (nrFiringsNeeded > demandList[srcActor->getId()])
    {
        // Update required number of firings of source actor
        demandList[srcActor->getId()] = nrFiringsNeeded;
        
        // Continue computing demand list from all channels incoming to source
        // actor
        for (SDFportsIter iter = srcActor->portsBegin();
                iter!= srcActor->portsEnd(); iter++)
        {
            SDFport *p = *iter;
            
            // Incoming channel?
            if (p->getType() == SDFport::In)
            {
                SDFchannel *c = p->getChannel();
                
                computeDemandList(c, demandList);
            }
        }
    }
}

/**
 * analyze ()
 * Compute the latency of an SDF graph for unconstrained buffer sizes and
 * using auto-concurrency using a state-space traversal and self-timed 
 * execution.
 */
void SDFstateSpaceMinimalLatencyAnalysis::analyze(TimedSDFgraph *g, 
        SDFactor *srcActor, SDFactor *dstActor, TDtime &latency, 
        TDtime &throughput)
{
    TimedSDFactor *dstDummyAct, *srcDummyAct;
    TransitionSystem *transitionSystem;
    TimedSDFactor::Processor proc;
    RepetitionVector repVec;
    TimedSDFchannel *chDummySrc, *chDummyDst, *ch;
    TimedSDFgraph *gr;
    vector<uint> demandList;

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(g))
    {
        throw CException("[ERROR] Graph is not strongly connected.");
    }

    // Compute repetition vector of the SDFG
    repVec = computeRepetitionVector(g);

    // Create a copy of the graph (the graph is modified in the function!)
    SDFcomponent comp(g->getParent(), g->getName(), g->getId());
    gr = g->clone(comp);

    // Locate src and dst actor in copy
    srcActor = gr->getActor(srcActor->getId());
    dstActor = gr->getActor(dstActor->getId());

    // Add source and destination dummy actors
    srcDummyAct = gr->createActor();
    dstDummyAct = gr->createActor();
    proc.type = "default";
    proc.execTime = 0;
    proc.stateSize = 0;
    srcDummyAct->addProcessor(&proc);
    srcDummyAct->setDefaultProcessor("default");
    dstDummyAct->addProcessor(&proc);
    dstDummyAct->setDefaultProcessor("default");

    // Connect dummy actors to real source and destination actors
    chDummySrc = gr->createChannel(srcDummyAct, repVec[srcActor->getId()],
                                        srcActor, 1, 0);
    chDummyDst = gr->createChannel(dstActor, 1, dstDummyAct,
                                        repVec[dstActor->getId()], 0);

    // Add self-loop to dummy source actor in graph gr with no tokens
    ch = gr->createChannel(srcDummyAct, 1, srcDummyAct, 1, 0);

    // Create a transition system
    transitionSystem = new TransitionSystem(gr);

    // Execute graph till deadlock
    transitionSystem->execSDFgraph();
    
    // Compute demand list
    demandList.resize(gr->nrActors());
    demandList[dstDummyAct->getId()] = 1;
    transitionSystem->computeDemandList(chDummyDst, demandList);

    #ifdef VERBOSE
    for (SDFactorsIter iter = gr->actorsBegin();
            iter != gr->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;

        cerr << "demand[" << a->getId() << "] = ";
        cerr << demandList[a->getId()] << endl;
    }
    #endif
    
    // Fire the source actor once (i.e. put tokens on its output channel)
    TransitionSystem::State &currentState = transitionSystem->getCurrentState();
    currentState.ch[chDummySrc->getId()] += repVec[srcActor->getId()];
    demandList[srcDummyAct->getId()] = 0;
    
    // Execute demand list in self-timed manner
    latency = transitionSystem->execSDFgraphUsingDemandList(demandList);
    
    // Throughput computation (first execute till deadlock)
    transitionSystem->execSDFgraph(true);
    
    // Fire the source actor once (i.e. put tokens on its output channel)
    currentState = transitionSystem->getCurrentState();
    currentState.ch[chDummySrc->getId()] += repVec[srcActor->getId()];
    
    // Continue the execution of the SDFG till a recurrent state is found. This
    // will be the state in which the SDFG deadlocks (requiring the source actor
    // to fire again). The function returns the time between the two deadlocks
    // (i.e. the inverse of the throughput). In this time, the source actor
    // has fired once (note that its entry in the repetition vector is also one)
    throughput = transitionSystem->execSDFgraph(false);
    throughput = 1.0 / throughput;
    
    // Cleanup
    delete transitionSystem;
    delete gr;
}    
    
/**
 * analyzeSingleProc ()
 * Compute the latency of an SDF graph for unconstrained buffer sizes and
 * using auto-concurrency using a state-space traversal and self-timed 
 * execution.
 */
void SDFstateSpaceMinimalLatencyAnalysis::analyzeSingleProc(TimedSDFgraph *g,
        SDFactor *srcActor, SDFactor *dstActor, TDtime &latency)
{
    TimedSDFactor *dstDummyAct, *srcDummyAct;
    TransitionSystem *transitionSystem;
    TimedSDFactor::Processor proc;
    RepetitionVector repVec;
    TimedSDFchannel *chDummySrc, *chDummyDst, *ch;
    TimedSDFgraph *gr;
    vector<uint> demandList;

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(g))
    {
        throw CException("[ERROR] Graph is not strongly connected.");
    }

    // Compute repetition vector of the SDFG
    repVec = computeRepetitionVector(g);

    // Create a copy of the graph (the graph is modified in the function!)
    SDFcomponent comp(g->getParent(), g->getName(), g->getId());
    gr = g->clone(comp);

    // Locate src and dst actor in copy
    srcActor = gr->getActor(srcActor->getId());
    dstActor = gr->getActor(dstActor->getId());

    // Add source and destination dummy actors
    srcDummyAct = gr->createActor();
    dstDummyAct = gr->createActor();
    proc.type = "default";
    proc.execTime = 1;
    proc.stateSize = 0;
    srcDummyAct->addProcessor(&proc);
    srcDummyAct->setDefaultProcessor("default");
    dstDummyAct->addProcessor(&proc);
    dstDummyAct->setDefaultProcessor("default");

    // Connect dummy actors to real source and destination actors
    chDummySrc = gr->createChannel(srcDummyAct, repVec[srcActor->getId()],
                                        srcActor, 1, 0);
    chDummyDst = gr->createChannel(dstActor, 1, dstDummyAct,
                                        repVec[dstActor->getId()], 0);

    // Add self-loop to dummy source actor in graph gr with no tokens
    ch = gr->createChannel(srcDummyAct, 1, srcDummyAct, 1, 0);

    // Create a transition system
    transitionSystem = new TransitionSystem(gr);

    // Execute graph till deadlock
    transitionSystem->execSDFgraph();
    
    // Compute demand list
    demandList.resize(gr->nrActors());
    demandList[dstDummyAct->getId()] = 1;
    transitionSystem->computeDemandList(chDummyDst, demandList);

    // Dummy actor no longer need to fire
    demandList[srcDummyAct->getId()] = 0;
    demandList[dstDummyAct->getId()] = 0;
    
    // Single processor latency is equal to the sum of the execution times of
    // all demanded actor firings
    latency = 0;
    for (SDFactorsIter iter = g->actorsBegin(); iter!= g->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*iter);
        
        latency = latency + demandList[a->getId()] * a->getExecutionTime();
    }

    // Cleanup
    delete transitionSystem;
    delete gr;
}
