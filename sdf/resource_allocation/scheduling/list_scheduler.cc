/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   list_scheduler.cc
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
 * $Id: list_scheduler.cc,v 1.5 2008/03/06 13:59:05 sander Exp $
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

#include "list_scheduler.h"
#include "../../base/algo/components.h"
#include "../../base/algo/repetition_vector.h"

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * clear ()
 * The function sets the state to zero.
 */
void SDFstateSpaceListScheduler::TransitionSystem::State::clear()
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
bool SDFstateSpaceListScheduler::TransitionSystem::State::operator==(
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

/**
 * print ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceListScheduler::TransitionSystem::State::print(ostream &out)
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
void SDFstateSpaceListScheduler::TransitionSystem::initOutputActor()
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
 * initStaticOrderSchedules ()
 * The function assigns empty static-order schedules to the processors.
 */
void SDFstateSpaceListScheduler::TransitionSystem::initStaticOrderSchedules()
{
    StaticOrderSchedule s;
    
    for (uint t = 0; t < bindingAwareSDFG->nrTilesInPlatformGraph(); t++)
        bindingAwareSDFG->getScheduleOnTile(t).clear();
}

/**
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the state s is not stored. The function
 * returns false. The function always sets the pos variable to the position
 * where the state s is in the list.
 */
bool SDFstateSpaceListScheduler::TransitionSystem::storeState(State &s,
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
TDtime SDFstateSpaceListScheduler::TransitionSystem::computeThroughput(
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
bool SDFstateSpaceListScheduler::TransitionSystem::actorReadyToFire(SDFactor *a)
{
    // Actor bound to processor?
    if (bindingAwareSDFG->getBindingOfActorToTile(a) != ACTOR_NOT_BOUND)
    {
        uint p = bindingAwareSDFG->getBindingOfActorToTile(a);
        
        // Processor is not idle or actor is not first in readyList?
        if (!procIdle[p] || actorReadyList[p].empty() 
                || actorReadyList[p].front()->getId() != a->getId())
        {
            return false;
        }
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
void SDFstateSpaceListScheduler::TransitionSystem::startActorFiring(
        TimedSDFactor *a)
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
        
        // Processor is no longer idle
        procIdle[p] = false;
        
        // Remove actor from the list of ready actors
        actorReadyList[p].pop_front();

        // Add actor to schedule of the processor
        bindingAwareSDFG->getScheduleOnTile(p).appendActor(a);

        // Store last point of schedule in the state
        SOS_POS(p) = bindingAwareSDFG->getScheduleOnTile(p).size() - 1;
    }

    // Add actor firing to the list of active firings of this actor
    currentState.actClk[a->getId()].push_back(completionTime);
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceListScheduler::TransitionSystem::actorReadyToEnd(SDFactor *a)
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
void SDFstateSpaceListScheduler::TransitionSystem::endActorFiring(SDFactor *a)
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

        // Processor becomes idle
        procIdle[p] = true;
    }    
}

/**
 * clockStep ()
 * The function progresses time till the first end of firing transition
 * becomes enabled. The time step is returned. In case of deadlock, the
 * time step is equal to UINT_MAX.
 */
SDFtime SDFstateSpaceListScheduler::TransitionSystem::clockStep()
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
 * execSDFgraph()  
 * Execute the SDF graph till a deadlock is found or a recurrent state.  
 * The throughput is returned.  
 */  
TDtime SDFstateSpaceListScheduler::TransitionSystem::execSDFgraph()  
{
    StatesIter recurrentState;
    SDFtime clkStep;
    int repCnt = 0;  

    // Clear the list of stored states
    clearStoredStates();

    // Initialize processor states
    actorReadyList.resize(bindingAwareSDFG->nrTilesInPlatformGraph());
    procIdle.resize(bindingAwareSDFG->nrTilesInPlatformGraph());
    for (uint p = 0; p <bindingAwareSDFG->nrTilesInPlatformGraph(); p++)
    {
        actorReadyList[p].clear();
        procIdle[p] = true;
    }
   
    // Create initial state
    currentState.init(bindingAwareSDFG->nrActors(), 
                        bindingAwareSDFG->nrChannels(), 
                        bindingAwareSDFG->nrTilesInPlatformGraph());
    currentState.clear();  
    previousState.init(bindingAwareSDFG->nrActors(), 
                        bindingAwareSDFG->nrChannels(), 
                        bindingAwareSDFG->nrTilesInPlatformGraph());
    previousState.clear();

    // Initial tokens and schedules
    for (SDFchannelsIter iter = bindingAwareSDFG->channelsBegin();
            iter != bindingAwareSDFG->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        
        CH(c->getId()) = c->getInitialTokens();
    }
    for (uint p = 0; p < bindingAwareSDFG->nrTilesInPlatformGraph(); p++)
    {
        SOS_POS(p) = 0;
        TDMA_POS(p) = 0;
    }
   
    // Fire the actors  
    while (true)  
    {
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
                            // Complete cycle in each processor schedule
                            for (uint p = 0; 
                                    p < bindingAwareSDFG->nrTilesInPlatformGraph(); 
                                        p++)
                            {
                                if (SOS(p).size() > 1)
                                {
                                    // Last schedule entry should be removed and
                                    // on the one before the last must loop-back 
                                    // to the schedule position in recurrent 
                                    // state.
                                    SOS(p).erase(--SOS(p).end());
                                    SOS(p).setStartPeriodicSchedule(
                                                recurrentState->schedulePos[p]);
                                }
                                else if (SOS(p).size() == 1)
                                {
                                    // Single state schedule
                                    SOS(p).setStartPeriodicSchedule(0);
                                }
                            }
                            
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

        // Update the actorReadyList with all firings that became enabled
        // after last clock step
        for (SDFactorsIter iter = bindingAwareSDFG->actorsBegin();
                iter != bindingAwareSDFG->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)(*iter);

            if (bindingAwareSDFG->getBindingOfActorToTile(a) != ACTOR_NOT_BOUND)
            {
                uint t = bindingAwareSDFG->getBindingOfActorToTile(a);
                uint nrFiringsCurrent = INT_MAX;
                uint nrFiringsPrevious = INT_MAX;
                uint nrFiringsEnabled;
                
                // Compute number of enabled firings in current 
                // and previous state
                for (SDFportsIter iterP = a->portsBegin();
                        iterP != a->portsEnd(); iterP++)
                {
                    SDFport *p = *iterP;
                    SDFchannel *c = p->getChannel();
                    uint nrCurrent, nrPrevious;
                    
                    if (p->getType() == SDFport::In)
                    {
                        nrCurrent = currentState.ch[c->getId()] / p->getRate();
                        nrPrevious = previousState.ch[c->getId()] /p->getRate();
                        
                        if (nrCurrent < nrFiringsCurrent)
                            nrFiringsCurrent = nrCurrent;
                        if (nrPrevious < nrFiringsPrevious)
                            nrFiringsPrevious = nrPrevious;
                    }
                }
                
                if (nrFiringsCurrent <= nrFiringsPrevious)
                    nrFiringsEnabled = 0;
                else
                    nrFiringsEnabled = nrFiringsCurrent - nrFiringsPrevious;
                
                // Add as many firings to the actorReadyList as became enabled
                for (uint i = 0; i < nrFiringsEnabled; i++)
                    actorReadyList[t].push_back(a);
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
            throw CException("Graph is not deadlock free.");
            return 0;
        }

        // Store partial state to check for progress
        for (uint i = 0; i < bindingAwareSDFG->nrChannels(); i++)
        {
            previousState.ch[i] = currentState.ch[i];
        }
    }
    
    return 0;
}  

/**
 * List scheduler
 * Constructs a set of static-order schedules for the processors in the
 * architecture platform. Each schedule orders the firings of the actors
 * running on the processor. A list scheduler is used to construct the
 * static-order schedules.
 */
void SDFstateSpaceListScheduler::schedule(BindingAwareSDFG *bg)
{
    // Check that the application graph is a strongly connected graph
    if (!isStronglyConnectedGraph(bg))
        throw CException("Graph is not strongly connected.");

    // Create a transition system
    TransitionSystem transitionSystem(bg);

    // Execute the graph to construct a schedule
    transitionSystem.execSDFgraph();
}
