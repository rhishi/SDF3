/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tdma_schedule.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 24, 2006
 *
 *  Function        :   Throughput calculation for SDFG mapped to an MPSoC 
 *                      with TDMA arbitration on the processors and a
 *                      static-order schedule for actors running on the same
 *                      processor.
 *
 *  History         :
 *      24-04-06    :   Initial version.
 *
 * $Id: tdma_schedule.cc,v 1.2 2008/03/06 13:59:05 sander Exp $
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

#include "tdma_schedule.h"
#include "../../base/algo/repetition_vector.h"
#include "../../base/algo/components.h"

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

//#define _PRINT_STATESPACE

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * clear ()
 * The function sets the state to zero.
 */
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem::State
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
bool SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem::State
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
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem::State
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
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
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
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
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
bool SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem::storeState(
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
TDtime SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
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

/**
 * computeTileUtilization ()
 * The function calculates the fraction of the time that a processor is busy
 * during the periodic part of the execution.
 */
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
    ::computeTileUtilization(const StatesIter recurrentState,
            vector<double> &tileUtilization)
{
    RepetitionVector repVec = computeRepetitionVector(bindingAwareSDFG);
	double nrItersInPeriod = 0;
	double lengthOfPeriod = 0;  

    // Initialize tile utilizations
    tileUtilization.resize(bindingAwareSDFG->nrTilesInPlatformGraph());
    for (uint t = 0; t < bindingAwareSDFG->nrTilesInPlatformGraph(); t++)
        tileUtilization[t] = 0;
    
	// Check all state from stack till cycle complete  
	for (StatesIter iter = recurrentState;
            iter != storedStates.end(); iter++)
    {
        State &s = *iter;

        // Number of states in cycle is equal to number of iterations 
        // in the period
        nrItersInPeriod++;

	    // Time between previous state  
	    lengthOfPeriod += s.glbClk;
	}
    
    // The activity of a processor is given by the sum of execution time of the
    // actors executed on the processor multiplied with the number of
    // invocations of an actor per iteration and the number of iterations per
    // period.
    for (SDFactorsIter iter = bindingAwareSDFG->actorsBegin();
            iter != bindingAwareSDFG->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*iter);
        uint t = bindingAwareSDFG->getBindingOfActorToTile(a);
        
        if (t != ACTOR_NOT_BOUND)
        {
            tileUtilization[t] = tileUtilization[t]
                 + a->getExecutionTime() * repVec[a->getId()] * nrItersInPeriod;
        }
    }
    
    // Normalize length of execution on processors with length of the period
    for (uint t = 0; t < bindingAwareSDFG->nrTilesInPlatformGraph(); t++)
        tileUtilization[t] = tileUtilization[t] / lengthOfPeriod;
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
bool SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
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
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
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
    
#ifdef _PRINT_STATESPACE
    cout << "start: " << a->getName() << endl;
#endif
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
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
void SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
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

#ifdef _PRINT_STATESPACE
    cout << "end:   " << a->getName() << endl;
#endif
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
SDFtime SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
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

#ifdef _PRINT_STATESPACE
    cout << "clk:   " << step << endl;
#endif

    return step;
}

/**  
 * execSDFgraph()  
 * Execute the SDF graph till a deadlock is found or a recurrent state.  
 * The throughput is returned.  
 */  
TDtime SDFstateSpaceBindingAwareThroughputAnalysis::TransitionSystem
    ::execSDFgraph(vector<double> &tileUtilization)  
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
        SDFchannel *c = *iter;
        
        CH(c->getId()) = c->getInitialTokens();
    }
    
    // Initial schedules
    for (uint p = 0; p < bindingAwareSDFG->nrTilesInPlatformGraph(); p++)
    {
        SOS_POS(p) = 0;
        TDMA_POS(p) = 0;
    }

#ifdef _PRINT_STATESPACE
    cout << "### start statespace exploration" << endl;
#endif

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
#ifdef _PRINT_STATESPACE
                        currentState.print(cout);
#endif
                        // Add state to hash of visited states  
                        if (!storeState(currentState, recurrentState))
                        {
                            computeTileUtilization(recurrentState,
                                                        tileUtilization);
                            
#ifdef _PRINT_STATESPACE
                            cout << "### end statespace exploration" << endl;
#endif

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
#ifdef _PRINT_STATESPACE
            cout << "Deadlock!" << endl;
            cout << "### end statespace exploration" << endl;
#endif
            return 0;
        }
    }
    
    return 0;
}  

/**
 * stateSpaceThroughputAnalysisBoundedSDFG ()
 * The function computes the throughput of a binding-aware SDFG bg. It assumes
 * that the platform uses a TDMA resource arbitration mechanism on the 
 * processors. An Actor is scheduled on a processor if its input tokens are 
 * available and the static-order schedule indicates that the actor is allowed 
 * to fire. Actors which are not mapped to a processor, only wait for their 
 * input tokens.
 */
double SDFstateSpaceBindingAwareThroughputAnalysis::analyze(
        BindingAwareSDFG *bg, vector<double> &tileUtilization)
{
    double thr;
    
    // Check that the application graph is a strongly connected graph
    if (!isStronglyConnectedGraph(bg))
        throw CException("Graph is not strongly connected.");

    // Create a transition system
    TransitionSystem transitionSystem(bg);
    
    // Find the maximal throughput
    thr = transitionSystem.execSDFgraph(tileUtilization);

    return thr;
}
