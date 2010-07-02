/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   throughput.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 5, 2006
 *
 *  Function        :   State-space based throughput analysis
 *
 *  History         :
 *      05-04-06    :   Initial version.
 *
 * $Id: throughput.cc,v 1.1.1.1 2007/10/02 10:59:49 sander Exp $
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

#include <math.h>  
#include <iostream>  
#include <assert.h>  
#include <time.h>  
#include <sys/resource.h>  

#include "statespace.h"
#include "../../base/algo/components.h"

#define TDTIME_MAX INT_MAX

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

typedef unsigned short TCnt;
typedef unsigned long TBufSize;
typedef double TDtime;

static TimedCSDFgraph *g;
static CId outputActor;
static TCnt outputActorRepCnt;
static uint period;

static
CId findOutputActor(TimedCSDFgraph *g,
        CSDFgraph::RepetitionVector repVec)
{
    int min = INT_MAX;
    CSDFactor *a = NULL;
    
    for (CSDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        if (repVec[(*iter)->getId()] < min)
        {
            a = *iter;
            min = repVec[a->getId()];
        }
    }
    
    return a->getId();
}

/******************************************************************************
 * State
 *****************************************************************************/

typedef struct _StateActorFiring
{
    CSDFtime execTime;
    TCnt  seqPos;
} StateActorFiring;

bool operator==(const StateActorFiring &s1, const StateActorFiring &s2)
{
    if (s1.execTime != s2.execTime || s1.seqPos != s2.seqPos)
        return false;
        
    return true;
}

class State
{
public:
    // Constructor
    State(const uint nrActors, const uint nrChannels) { 
        init(nrActors, nrChannels);
    };
    
    // Destructor
    ~State(){};

    // Initialize the state    
    void init(const uint nrActors, const uint nrChannels)
    {
        actClk.resize(nrActors);
        actSeqPos.resize(nrActors);
        ch.resize(nrChannels);
    };
    
    // State information
    vector< list<StateActorFiring> > actClk;
    vector< TCnt > actSeqPos;
    vector< TBufSize > ch;
    unsigned long glbClk;
};

#if 0
/**
 * printState ()
 * Print the state to the supplied stream.
 */
static
void printState(const State &s, ostream &out)
{
    out << "### State ###" << endl;

    for (uint i = 0; i < g->nrActors(); i++)
    {
        out << "actClk[" << i << "] =";
        
        for (list<StateActorFiring>::const_iterator iter = s.actClk[i].begin();
                iter != s.actClk[i].end(); iter++)
        {
            out << " (" << (*iter).execTime << ", ";
            out << (*iter).seqPos << "),";
        }
        
        out << endl;
    }

    for (uint i = 0; i < g->nrActors(); i++)
    {
        out << "actSeqPos[" << i << "] = " << s.actSeqPos[i] << endl;
    }
    
    for (uint i = 0; i < g->nrChannels(); i++)
    {
        out << "ch[" << i << "] = " << s.ch[i] << endl;
    }

    out << "glbClk = " << s.glbClk << endl;
}
#endif

/**
 * clearState ()
 * The function sets the state to zero.
 */
static
void clearState(State &s)
{
    for (uint i = 0; i < g->nrActors(); i++)
    {
        s.actClk[i].clear();
        s.actSeqPos[i] = 0;
    }

    for (uint i = 0; i < g->nrActors(); i++)
    {
        s.ch[i] = 0;
    }
    
    s.glbClk = 0;
}

/**
 * equalStates ()
 * The function compares to states and returns true if they are equal.
 */
inline
bool equalStates(const State &s1, const State &s2)
{
    if (s1.glbClk != s2.glbClk)
        return false;
    
    for (uint i = 0; i < g->nrChannels(); i++)
    {
        if (s1.ch[i] != s2.ch[i])
            return false;
    }
    
    for (uint i = 0; i < g->nrActors(); i++)
    {
        if (s1.actClk[i] != s2.actClk[i])
            return false;
        
        if (s1.actSeqPos[i] != s2.actSeqPos[i])
            return false;
    }
    
    return true;
}

/**
 * copyState ()
 * The function copies the state.
 */
inline
void copyState(State &to, const State &from)
{
    to.glbClk = from.glbClk;
    
    for (uint i = 0; i < g->nrChannels(); i++)
    {
        to.ch[i] = from.ch[i];
    }
    
    for (uint i = 0; i < g->nrActors(); i++)
    {
        to.actClk[i] = from.actClk[i];
        to.actSeqPos[i] = from.actSeqPos[i];
    }
}

/******************************************************************************
 * States
 *****************************************************************************/

typedef list<State>         States;
typedef States::iterator    StatesIter;

/**
 * storedStates
 * List of visited states that are stored.
 */
static States storedStates;

/**
 * storeState ()
 * The function stores the state s on whenever s is not already in the
 * list of storedStates. When s is stored, the function returns true. When the
 * state s is already in the list, the state s is not stored. The function
 * returns false. The function always sets the pos variable to the position
 * where the state s is in the list.
 */
static
bool storeState(State &s, StatesIter &pos)
{
    // Find state in the list of stored states
    for (StatesIter iter = storedStates.begin();
            iter != storedStates.end(); iter++)
    {
        State &x = *iter;
        
        // State s at position iter in the list?
        if (equalStates(x, s))
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
 * clearStoredStates ()
 * The function clears the list of stored states.
 */
static
void clearStoredStates()
{
    storedStates.clear();
}

/******************************************************************************
 * SDF
 *****************************************************************************/

#define CH(c)               currentState.ch[c]
#define CH_TOKENS(c,n)      (CH(c) >= n)  
#define CONSUME(c,n)        CH(c) = CH(c) - n;
#define PRODUCE(c,n)        CH(c) = CH(c) + n;

#define ACT_SEQ_POS(a)      currentState.actSeqPos[a]
#define ADV_ACT_SEQ_POS(a)  ACT_SEQ_POS(a) = (ACT_SEQ_POS(a) + 1) % period;

#define CH_TOKENS_PREV(c,n) (previousState.ch[c] >= n)  

static State currentState(0,0);  
static State previousState(0,0);  

/**  
 * computeThroughput ()  
 * The function calculates the throughput of the states on the cycle. Its  
 * value is equal to the average number of firings of an actor per time unit.
 */  
static inline  
TDtime computeThroughput(const StatesIter cycleIter)  
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
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
inline
bool actorReadyToFire(CSDFactor *a)
{
    // Check all input ports for tokens
    for (CSDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        CSDFport *p = *iter;
        CSDFchannel *c = p->getChannel();
        
        // Actor is destination of the channel?
        if (p->getPortType() == CSDFport::In)
        {
            if (!CH_TOKENS(c->getId(),
                        p->getRate().getRate(ACT_SEQ_POS(a->getId()))))
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
inline
void startActorFiring(TimedCSDFactor *a)
{
    StateActorFiring actFiring;
    bool addedFiring = false;

    // Set properties of this actor firing
    actFiring.execTime = a->getExecutionTime().value(ACT_SEQ_POS(a->getId()));
    actFiring.seqPos = ACT_SEQ_POS(a->getId());
    
    // Consume tokens from inputs and space for output tokens
    for (CSDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        CSDFport *p = *iter;
        CSDFchannel *c = p->getChannel();
        
        // Actor is destination of the channel?
        if (p->getPortType() == CSDFport::In)
        {
            CONSUME(c->getId(), p->getRate().getRate(ACT_SEQ_POS(a->getId())));
        }
    }

    // Add actor firing to the list of active firings of this actor
    for (list<StateActorFiring>::iterator iter =
                currentState.actClk[a->getId()].begin();
                    iter != currentState.actClk[a->getId()].end(); iter++)
    {
        if ((*iter).execTime >= actFiring.execTime)
        {
            currentState.actClk[a->getId()].insert(iter, actFiring);
            addedFiring = true;
            break;
        }
    }
    if (!addedFiring)
    {
        // Not yet inserted, put it at the end
        currentState.actClk[a->getId()].push_back(actFiring);
    }

    // Advance the sequence position of the actor
    ADV_ACT_SEQ_POS(a->getId());
}

/**
 * actorReadyToEnd ()
 * The function returns true when the actor is ready to end its firing. Else
 * the function returns false.
 */
inline
bool actorReadyToEnd(CSDFactor *a)
{
    if (currentState.actClk[a->getId()].empty())
        return false;
    
    // First actor firing in sorted list has execution time left?
    if (currentState.actClk[a->getId()].front().execTime != 0)
        return false;

    return true;
}

/**
 * endActorFiring ()
 * Produce tokens on all output channels and remove the actor firing from the
 * list of active firings.
 */
inline
void endActorFiring(CSDFactor *a)
{
    StateActorFiring &actFiring = currentState.actClk[a->getId()].front();

    for (CSDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        CSDFport *p = *iter;
        CSDFchannel *c = p->getChannel();
        
        // Actor is source of the channel?
        if (p->getPortType() == CSDFport::Out)
        {
            PRODUCE(c->getId(), p->getRate().getRate(actFiring.seqPos));
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
inline
CSDFtime clockStep()
{
    CSDFtime step = UINT_MAX;
    
    // Find maximal time progress
    for (uint a = 0; a < g->nrActors(); a++)
    {
        if (!currentState.actClk[a].empty())
        {
            CSDFtime actClk = currentState.actClk[a].front().execTime;

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
        for (list<StateActorFiring>::iterator
                iter = currentState.actClk[a].begin();
                    iter != currentState.actClk[a].end(); iter++)
        {
            StateActorFiring &actFiring = *iter;
            
            // Lower remaining execution time of the actor firing
            actFiring.execTime -= step;
        }
    }

    // Advance the global clock  
    currentState.glbClk += step;

    return step;
}

/**  
 * execCSDFgraph()  
 * Execute the CSDF graph till a deadlock is found or a recurrent state.  
 * The throughput is returned.  
 */  
static
TDtime execCSDFgraph()  
{
    StatesIter recurrentState;
    CSDFtime clkStep;
    int repCnt = 0;  

    // Clear the list of stored states
    clearStoredStates();
    
    // Create initial state
    currentState.init(g->nrActors(), g->nrChannels());
    clearState(currentState);  
    previousState.init(g->nrActors(), g->nrChannels());
    clearState(previousState);  

    // Initial tokens and space
    for (CSDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        CSDFchannel *c = *iter;
        
        CH(c->getId()) = c->getInitialTokens();
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
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            CSDFactor *a = *iter;
            
            while (actorReadyToEnd(a))  
            {
                if (outputActor == a->getId())
                {
                    repCnt++;
                    if (repCnt == outputActorRepCnt)  
                    { 
                        // Add state to hash of visited states  
                        if (!storeState(currentState, recurrentState))
                        {
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
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedCSDFactor *a = (TimedCSDFactor*)(*iter);
        
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
 * stateSpaceThroughputAnalysis ()
 * Compute the throughput of an CSDF graph for unconstrained buffer sizes and
 * using auto-concurrency using a state-space traversal.
 */
extern
double stateSpaceThroughputAnalysis(TimedCSDFgraph *gr)
{
    CSDFgraph::RepetitionVector repVec;
    double thr;

    // Copy arguments to globals
    g = gr;

    // Compute repetition vector
    repVec = g->getRepetitionVector();
    
    // Check that the graph gr is a strongly connected graph
    if (!isStronglyConnectedGraph(gr))
    {
        // Split graph into its strongly connected components
        // and find minimal throughput of all components
        CSDFgraphComponents comp = stronglyConnectedComponents(gr);
        thr = TDTIME_MAX;
        
        for (CSDFgraphComponentsIter iter = comp.begin();
                iter != comp.end(); iter++)
        {
            CSDFgraph::RepetitionVector repVecGC;
            CSDFgraphComponent co = *iter;
            TimedCSDFgraph *gc;
            double thrGc;
            CId idFirstActor;
            CId id;
            
            // Construct graph from component
            gc = (TimedCSDFgraph*) componentToCSDFgraph(co);

            // Id of first actor in component
            idFirstActor = (*gc->actorsBegin())->getId();
            
            // Relabel actors
            id = 0;
            for (CSDFactorsIter iter = gc->actorsBegin(); 
                    iter != gc->actorsEnd(); iter++)
            {
                CSDFactor *u = *iter;

                u->setId(id);
                id++;
            }

            // Relabel channels
            id = 0;
            for (CSDFchannelsIter iter = gc->channelsBegin(); 
                    iter != gc->channelsEnd(); iter++)
            {
                CSDFchannel *c = *iter;

                c->setId(id);
                id++;
            }
            
            // Graph contains at least one channel
            if (gc->nrChannels() > 0)
            {
                // Compute throughput component
                thrGc = stateSpaceThroughputAnalysis(gc);

                // Compute repetition vector of component
                repVecGC = gc->getRepetitionVector();

                // Scale throughput wrt repetition vector component vs graph
                thrGc = (thrGc * repVecGC[0]) / (double)(repVec[idFirstActor]);

                if (thrGc < thr)
                    thr = thrGc;
            }
            
            // Cleanup
            delete gc;
        }

        return thr;
    }
    
    // Find the output actor and its repetition count
    outputActor = findOutputActor(g, repVec);
    outputActorRepCnt = repVec[outputActor];

    // Compute the period of the graph
    period = 0;
    
    // Check that all periods in the graph are equal
    for (CSDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        CSDFchannel *c = *iter;
        
        if (period == 0)
        {
            period = c->getSrcPort()->getRate().lengthRatesVector();
        }
        else
        {
            if (period != c->getSrcPort()->getRate().lengthRatesVector()
                    || period != c->getDstPort()->getRate().lengthRatesVector())
            {
                throw CException("All sequence must be of equal length.");
            }   
        }
    }
    for (CSDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        TimedCSDFactor *a = (TimedCSDFactor*)(*iter);
        
        if (period != a->getExecutionTime().size())
            throw CException("All sequence must be of equal length.");
    }
    
    // Find the maximal throughput
    thr = execCSDFgraph();
    
    return thr;
}
