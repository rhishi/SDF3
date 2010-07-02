/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   static_periodic_scheduler_chao.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   September 18, 2008
 *
 *  Function        :   Construct static-periodic schedule with maximal 
 *                      throughput using scheduling algorithm from L.-F. Chao
 *                      and E.H.-M. Sha in "Static scheduling of DSP algorithms
 *                      on various models".
 *
 *  History         :
 *      18-09-08    :   Initial version.
 *
 * $Id: static_periodic_scheduler_chao.cc,v 1.1 2008/09/25 10:49:58 sander Exp $
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

#include "static_periodic_scheduler_chao.h"
#include "../../base/algo/repetition_vector.h"
#include "../../base/algo/components.h"
#include "../../base/hsdf/check.h"

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

//#define _PRINT_STATESPACE

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::State::print(
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
void SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::State::clear()
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
bool SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::State::operator==(
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
void SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::initOutputActor()
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
bool SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::storeState(State &s,
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
CFraction SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem
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

	return CFraction(nr_fire, time);  
}

/******************************************************************************
 * SDF
 *****************************************************************************/

#define CH(c)               currentState.ch[c]
#define CH_TOKENS(c,n)      (CH(c) >= n)  
#define CONSUME(c,n)        CH(c) = CH(c) - n;
#define PRODUCE(c,n)        CH(c) = CH(c) + n;

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::actorReadyToFire(
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
void SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::startActorFiring(
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

#ifdef _PRINT_STATESPACE
    cout << "start: " << a->getName() << endl;
#endif
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
bool SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::actorReadyToEnd(
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
void SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::endActorFiring(
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
SDFtime SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem::clockStep(
    SDFtime step)
{
    // Find maximal time progress
    for (uint a = 0; a < g->nrActors(); a++)
    {
        SDFtime actClk;
        
        // Remaining execution time
        if (!currentState.actClk[a].empty())
        {
            actClk = currentState.actClk[a].front();

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

#ifdef _PRINT_STATESPACE
    cout << "clk:   " << step << endl;
#endif

    return step;
}

/**  
 * execSDFgraphStaticPeriodic()  
 * Execute the SDF graph following the supplied schedule. The function returns
 * the throughput of the graph under the given schedule or zero when the
 * schedule cannot be met. 
 */  
CFraction SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem
    ::execSDFgraphStaticPeriodic(
        const vector< vector<long long int> > &startTime,
        const long long int period)  
{
    vector< vector<long long int> > iterCnt;
    vector<long long int> firingIdx;
    long long int globalTime = 0;
    StatesIter recurrentState;
    SDFtime clkStep;
    int repCnt = 0;  

    // Initialize iteration count vectors and th index of next firing of 
    // every actor within the schedule
    iterCnt.resize(g->nrActors());
    firingIdx.resize(g->nrActors());
    for (uint i = 0; i < g->nrActors(); i++)
    {
        firingIdx[i] = 0;
        iterCnt[i].resize(startTime[i].size());
        for (uint j = 0; j < iterCnt[i].size(); j++)
        {
            iterCnt[i][j] = 0;
        } 
    }
    
    // Clear the list of stored states
    clearStoredStates();
    
    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();  

    // Initial tokens and space
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        
        CH(c->getId()) = c->getInitialTokens();
    }

#ifdef _PRINT_STATESPACE
    cout << "### start statespace exploration" << endl;
#endif

    // Fire the actors  
    while (true)  
    {
        // Finish actor firings  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            
            while (actorReadyToEnd(a))  
            {
                if (outputActor == a->getId())
                {
                    repCnt++;
                    if (repCnt == outputActorRepCnt)  
                    { 
#ifdef _PRINT_STATESPACE
                        printState(currentState, cout);
#endif
                        // Add state to hash of visited states  
                        if (!storeState(currentState, recurrentState))
                        {
#ifdef _PRINT_STATESPACE
                            cout << "### end statespace exploration" << endl;
#endif
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

            // Should next firing of actor a start now?
            while (startTime[a->getId()][firingIdx[a->getId()]] 
                    + period * iterCnt[a->getId()][firingIdx[a->getId()]]
                     <= globalTime)
            {
                // Ready to fire actor a?
                if (actorReadyToFire(a))
                {
                    // Fire actor a
                    startActorFiring(a);
                    iterCnt[a->getId()][firingIdx[a->getId()]]++;
                    firingIdx[a->getId()] = (firingIdx[a->getId()] + 1) 
                                                % startTime[a->getId()].size();
                }
                else
                {
                    // Actor not ready to fire, violation of schedule
                    return 0;
                }
            }
        }

        // Clock step
        clkStep = UINT_MAX;
        for (uint a = 0; a < g->nrActors(); a++)
        {
            SDFtime actClk;
            
            // Time till next point from which actor is allowed to fire
            if (startTime[a][firingIdx[a]] + period * iterCnt[a][firingIdx[a]] 
                    - globalTime > 0)
            {
                actClk = startTime[a][firingIdx[a]] 
                               + period * iterCnt[a][firingIdx[a]] - globalTime;
                if (clkStep > actClk)
                    clkStep = actClk;
            }
        }
        clkStep = clockStep(clkStep);
        globalTime += clkStep;
        
        // Deadlocked?
        if (clkStep == UINT_MAX)
        {
#ifdef _PRINT_STATESPACE
            cout << "### end statespace exploration" << endl;
#endif
            return 0;
        }
    }
    
    return 0;
}  

/**  
 * execSDFgraphSelfTimed()  
 * Execute the SDF graph till a deadlock is found or a recurrent state.  
 * The throughput is returned.  
 */  
CFraction SDFstateSpaceStaticPeriodicSchedulerChao::TransitionSystem
    ::execSDFgraphSelfTimed()
{
    StatesIter recurrentState;
    SDFtime clkStep;
    int repCnt = 0;  
    
    // Clear the list of stored states
    clearStoredStates();
    
    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();  

    // Initial tokens and space
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        
        CH(c->getId()) = c->getInitialTokens();
    }

#ifdef _PRINT_STATESPACE
    cout << "### start statespace exploration" << endl;
#endif

    // Fire the actors  
    while (true)  
    {
        // Finish actor firings  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            
            while (actorReadyToEnd(a))  
            {
                if (outputActor == a->getId())
                {
                    repCnt++;
                    if (repCnt == outputActorRepCnt)  
                    { 
#ifdef _PRINT_STATESPACE
                        printState(currentState, cout);
#endif
                        // Add state to hash of visited states  
                        if (!storeState(currentState, recurrentState))
                        {
#ifdef _PRINT_STATESPACE
                            cout << "### end statespace exploration" << endl;
#endif
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
#ifdef _PRINT_STATESPACE
            cout << "### end statespace exploration" << endl;
#endif
            return 0;
        }
    }
    
    return 0;
}  

/**  
 * execSDFgraphComputeSchedule()  
 * ECompute a static-periodic schedule using the algorithm from [Chao95].
 */  
void SDFstateSpaceStaticPeriodicSchedulerChao::computeSchedule(TimedSDFgraph *g,
        vector< vector<long long int> > &startTime, const long long int c,
        const long long int f)
{
    long long int pass;
    TimedSDFactor *v;
    SDFactor *last;
    double period;
    
    // Ratio of c and f is the period
    period = (double)(c) / (double)(f);
    
    // Initialize the shortest path and queue Q
    vector<double> sh(g->nrActors(), 0);
    SDFactors Q = g->getActors();
    
    pass = 0;
    last = Q.back();
    while (!Q.empty() && pass < g->nrActors())
    {
        v = (TimedSDFactor*)Q.front();
        Q.pop_front();

        // Iterate over all ports in the actor
        for (SDFportsIter iter = v->portsBegin();
                iter != v->portsEnd(); iter++)
        {
            SDFport *p = *iter;
            
            // Is the ports an output port?
            if (p->getType() == SDFport::Out)
            {
                SDFchannel *e = p->getChannel();
                SDFactor *w = e->getDstActor();
                double shVtoW = sh[v->getId()] + (double)(e->getInitialTokens()) 
                            - (double)(v->getExecutionTime()) * 1.0 / period;
                
                // Path from v to w shorter then currently known paths to w?
                if (sh[w->getId()] > shVtoW)
                {
                    // Update shortest path to w
                    sh[w->getId()] = shVtoW;
                
                    // Shortest path computation of w must be (re)done
                    if (find(Q.begin(), Q.end(), w) == Q.end() && v != w)
                    {
                        Q.push_back(w);
                    }
                }
            }
        }
        
        // Completed one iteration over all nodes whose shortest path length 
        // must be (re)computed
        if (v == last)
        {
            pass++;
            last = Q.back();
        }
    }
    
    // Found a valid schedule?
    if (Q.empty())
    {
        // Compute the actual schedule
        startTime.resize(g->nrActors());
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            v = (TimedSDFactor*)(*iter);
            startTime[v->getId()].resize(f);

            for (uint i = 0; i < f; i++)
            {

                startTime[v->getId()][i] = ceil(-sh[v->getId()] * period 
                                                                + i * period);
            }
        }
    }
    else
    {
        throw CException("[ERROR] No such optimal schedule exists.");
    }
}

/**
 * schedule ()
 * Compute a static-periodic schedule with maximal throughput for the supplied
 * SDFG.
 */
void SDFstateSpaceStaticPeriodicSchedulerChao::schedule(TimedSDFgraph *g)
{
    vector< vector<long long int> > startTime;
    long long int unfoldingFactor, cyclePeriod;
    CFraction thr, thrSchedule;
    
    // The graph must be strongly connected when constructing the schedule
    if (!isStronglyConnectedGraph(g))
        throw CException("[ERROR] The graph is not strongly connected.");

    // The graph must be a homogeneous SDFG
    if (!isHSDFgraph(g))
        throw CException("[ERROR] The graph is not an HSDFG.");

    // Create a transition system
    TransitionSystem transitionSystem(g);

    // Compute the maximal throughput of the graph
    thr = transitionSystem.execSDFgraphSelfTimed();

    // Reduce throughput fraction to irreducable form
    thr = thr.lowestTerm();

    // Compute the periodicity and period of the schedule
    unfoldingFactor = thr.numerator();
    cyclePeriod = thr.denominator();

    // Compute the schedule
    computeSchedule(g, startTime, cyclePeriod, unfoldingFactor);
    
    // Execute the schedule
    thrSchedule = transitionSystem.execSDFgraphStaticPeriodic(startTime,
                                                                   cyclePeriod);
    
    cout << "period: " << cyclePeriod << endl;
    for (uint i = 0; i < g->nrActors(); i++)
    {
        for (uint j = 0; j < startTime[i].size(); j++)
        {
            cout << "(" << g->getActor(i)->getName() << "," << j << "): " << startTime[i][j] << endl;
        }
    }
    cout << "thr: " << thrSchedule << endl;

    if (thr != thrSchedule)
    {
        cout << "optimal: " << thr << endl;
    }
}

