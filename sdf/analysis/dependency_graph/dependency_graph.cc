/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dependency_graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 20, 2006
 *
 *  Function        :   Find the abstract dependency graph for an SDFG
 *
 *  History         :
 *      20-06-06    :   Initial version.
 *
 * $Id: dependency_graph.cc,v 1.1 2008/03/06 10:49:42 sander Exp $
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

#include "dependency_graph.h"
#include "../../base/algo/repetition_vector.h"
#include "../../base/algo/components.h"

#define TDTIME_MAX INT_MAX

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

typedef unsigned short TCnt;
typedef unsigned long TBufSize;
typedef double TDtime;

static TimedSDFgraph *g;
static SDFactor *outputActor;
static SDFtime SDF_MAX_TIME;
static TCnt outputActorRepCnt;

static
SDFactor *findOutputActor(TimedSDFgraph *g, RepetitionVector repVec)
{
    int min = INT_MAX;
    SDFactor *a = NULL;
    
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        if (repVec[(*iter)->getId()] < min)
        {
            a = *iter;
            min = repVec[a->getId()];
        }
    }
    
    return a;
}

static
SDFtime getMaxExecTime(TimedSDFgraph *g)
{
    SDFtime max = 0;
    
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*iter);
        
        if (a->getExecutionTime() > max)
            max = a->getExecutionTime();
    }
    
    return max;
}

/******************************************************************************   * State  
 *****************************************************************************/
typedef struct _State  
{  
  TCnt **act_clk;
  TDtime glb_clk;  
  TBufSize *ch; 
} State;  

/**  
 * clearState ()  
 * The function sets the state to zero.  
 */  
static
void clearState(State &s)  
{
    for (uint i = 0; i < g->nrChannels(); i++)
        s.ch[i] = 0;
    
    s.glb_clk = 0;

    for (uint i = 0; i < g->nrActors(); i++)
        for (TCnt t = 0; t < SDF_MAX_TIME+1; t++)
            s.act_clk[i][t] = 0;
}  

/**  
 * equalStates ()  
 * The function compares to states and returns true if they are equal.  
 */  
static inline  
bool equalStates(const State &s1, const State &s2)  
{
    if (s1.glb_clk != s2.glb_clk)
        return false;
    
    for (uint i = 0; i < g->nrChannels(); i++)
        if (s1.ch[i] != s2.ch[i])
            return false;
    
    for (uint i = 0; i < g->nrActors(); i++)
        for (uint t = 0; t < SDF_MAX_TIME+1; t++)
            if (s1.act_clk[i][t] != s2.act_clk[i][t])
                return false;

    return true;
}  

/**  
 * copyState ()  
 * The function copies the state.  
 */  
static inline  
void copyState(State &to, const State &from)  
{  
    to.glb_clk = from.glb_clk;
    
    for (uint i = 0; i < g->nrChannels(); i++)
        to.ch[i] = from.ch[i];
    
    for (uint i = 0; i < g->nrActors(); i++)
        for (uint t = 0; t < SDF_MAX_TIME+1; t++)
            to.act_clk[i][t] = from.act_clk[i][t];
}

/**  
 * createState ()  
 * The function allocates memory for a new state.
 */  
static
void createState(State &s)  
{
    s.ch = new TBufSize [g->nrChannels()];
    
    s.act_clk = new TCnt* [g->nrActors()];
    for (uint i = 0; i < g->nrActors(); i++)
        s.act_clk[i] = new TCnt [SDF_MAX_TIME+1];
}  

/**  
 * destroyState ()  
 * The function deallocates all data structures of the state
 */  
static
void destroyState(State &s)  
{
    delete [] s.ch;
    
    for (uint i = 0; i < g->nrActors(); i++)
        delete [] s.act_clk[i];
    delete [] s.act_clk;
}  

/******************************************************************************
 * Stack
 *****************************************************************************/

/**  
 * StackPos  
 * Index into stack.  
 */  
typedef long long StackPos;  

/**  
 * stack  
 * The stack...  
 */  
static State *stack;  

/**  
 * stackPtr  
 * Index into stack of the first free space.  
 */  
static StackPos stackPtr;  
static StackPos maxStackPtr;  
static StackPos STACK_SIZE;  

/**
 * resizeStack ()
 * Enlarge the stack with a factor of 2.
 */
static
void resizeStack()
{
    StackPos newStackSize = 2 * STACK_SIZE;
    State *newStack;  

    // Allocate new stack
    newStack = new State [newStackSize];
    
    // Copy old stack to new stack
    for (StackPos p = 0; p < STACK_SIZE; p++)
        newStack[p] = stack[p];
    
    // Create state for new part of stack
    for (StackPos p = STACK_SIZE; p < newStackSize; p++)
        createState(newStack[p]);
    
    // Cleanup old stack
    delete [] stack;
    
    // Put new stack in place
    stack = newStack;
    STACK_SIZE = newStackSize;
}

/**  
 * createStack ()  
 * The function creates a stack.  
 */  
static
void createStack()  
{  
    stack = new State [STACK_SIZE];

    if (stack == NULL)
        throw CException("Failed creating stack.");  

    for (uint i = 0; i < STACK_SIZE; i++)
        createState(stack[i]);

    maxStackPtr = 0;  
}

/**  
 * destroyStack()  
 * The function destroys a stack.  
 */
static  
void destroyStack()  
{  
    for (uint i = 0; i < STACK_SIZE; i++)
        destroyState(stack[i]);

    delete [] stack;
}  

/**  
 * clearStack ()  
 * Reset the stack to an empty state.  
 */
static
void clearStack()  
{  
    stackPtr = 0;     
}  

/**  
 * popStack ()  
 * Remove last element from the stack.  
 */  
static inline  
void popStack()  
{  
    if (stackPtr > 0)  
	    stackPtr--;  
}  

/**  
 * pushStack ()  
 * Put an element on the stack.  
 */  
static inline  
void pushStack(const State &s)  
{  
    if (stackPtr >= STACK_SIZE)
    {
        cerr << "[INFO] buffy resizing stack." << endl;
        resizeStack();
    }
    
    copyState(stack[stackPtr], s);
    stackPtr++;

    if (stackPtr > maxStackPtr) maxStackPtr = stackPtr;
}  

/**  
 * topStack ()  
 * The function returns the last element of the stack.  
 */  
static inline  
State &topStack()  
{  
    return stack[stackPtr-1];  
}  
      /******************************************************************************
 * Hash
 *****************************************************************************/  
#define INVALID_HASH_KEY    NULL  
#define NO_SLOT_IN_HASH     -1  
	    
/**  
 * HashKey  
 *  
 */  
typedef unsigned long long HashKey;  

/**  
 * HashSlot  
 *  
 */  
typedef struct _HashSlot  
{  
	StackPos value;  
	struct _HashSlot *next;  
} HashSlot;  

/**  
 * hashTable  
 * The hash table...  
 */  
static HashSlot **hashTable;  
static HashKey HASH_TABLE_SIZE;

/**  
 * createHashTable ()  
 * The function constructs a hash table.  
 */  
static
void createHashTable()  
{  
	hashTable = new HashSlot* [HASH_TABLE_SIZE];

	if (hashTable == NULL)
        throw CException("Failed creating hash table.");  

	for (uint i = 0; i < HASH_TABLE_SIZE; i++)  
	    hashTable[i] = INVALID_HASH_KEY;  
}

/**  
 * destroyHashTable ()  
 * The function destoys a hash table.  
 */  
static
void destroyHashTable()  
{  
	HashSlot *s_cur, *s_next;  

	for (uint i = 0; i < HASH_TABLE_SIZE; i++)  
	{  
	    for (s_cur = hashTable[i]; s_cur != INVALID_HASH_KEY; s_cur = s_next)  
	    {  
	        s_next = s_cur->next;  
	        delete s_cur;  
	    }  
	}  

	delete [] hashTable;  
}  
	    
/**  
 * clearHashTable ()  
 * Resets the hash table to contain no keys.  
 */  
static
void clearHashTable()  
{  
	HashSlot *s_cur, *s_next;  

	for (uint i = 0; i < HASH_TABLE_SIZE; i++)  
	{  
	    for (s_cur = hashTable[i]; s_cur != INVALID_HASH_KEY; s_cur = s_next)  
	    {  
	        s_next = s_cur->next;
            delete s_cur;  
	    }  

	    hashTable[i] = INVALID_HASH_KEY;  
	}  
}  
	    
/**  
 * hash ()  
 * The hash function. It is a standard multiplication hash.  
 */  
static inline  
HashKey hash(const State &s)  
{  
	double key = 0;  
	long long key_part;  

    key_part = (long long)(s.glb_clk);
    key = key * 39164205.20662217 + double(key_part) * 0.6180339887;
    
    for (uint i = 0; i < g->nrActors(); i++)
    {
        for (TCnt t = 0; t < SDF_MAX_TIME+1; t++)
        {
            key_part = (long long)(s.act_clk[i][t]);
    	    key = key * 39164205.20662217 + double(key_part) * 0.6180339887;
        }
    }

    for (uint i = 0; i < g->nrChannels(); i++)
    {
        key_part = (long long)(s.ch[i]);
	    key = key * 39164205.20662217 + double(key_part) * 0.6180339887;
    }

	key = fmod(key, 1);  
	return (HashKey) floor(HASH_TABLE_SIZE * key);  
}  
	    
/**  
 * insertKeyHashTable ()  
 * The function inserts a (key, value) pair into the hash table.  
 */  
static inline  
void insertKeyHashTable(const HashKey key, const StackPos value)  
{  
	HashSlot *slot = new HashSlot;  
	slot->value = value;
	slot->next = hashTable[key];  

	// Insert the stack position in the hash table  
	hashTable[key] = slot;  
}  
	    
/**  
 * findVallueInHashTable ()  
 * The function returns the StackPos at which the given (key,state) pair is  
 * located or NO_SLOT_IN_HASH if unkown.  
 */  
static inline  
StackPos findValueInHashTable(const HashKey key, const State &s)  
{  
	HashSlot *slot;  

	for (slot = hashTable[key]; slot != INVALID_HASH_KEY; slot = slot->next)  
	{  
	    StackPos pos = slot->value;  

	    if (equalStates(stack[pos], s))  
	        return pos;  
	}  

	return NO_SLOT_IN_HASH;  
}  

/******************************************************************************
 * Dependencies
 *****************************************************************************/

/**
 * curReachableNodes
 * The array indicates wether an actor b in the current period has a chain
 * of dependencies to an actor a in the previous period. If there is
 * such a dependency the value of curReachableNodes[b][a] == 1, else it is 0.
 *
 * The 'curReachableNodesCh' array maintains the information which back channels
 * are seen on at least one of the paths from b to a.
 *
 * The 'prev' version of both arrays are used during the dependency construction
 * process.
 */
static bool **curReachableNodes;
static bool **prevReachableNodes;

static bool ***curReachableNodesCh;
static bool ***prevReachableNodesCh;

/**
 * createDependencies ()
 * The function creates an initial abstract dependency graph.
 */
static 
void createDependencies()
{
    curReachableNodes = new bool* [g->nrActors()];
    prevReachableNodes = new bool* [g->nrActors()];
    curReachableNodesCh = new bool** [g->nrActors()];
    prevReachableNodesCh = new bool** [g->nrActors()];

    for (uint a = 0; a < g->nrActors(); a++)
    {
        curReachableNodes[a] = new bool [g->nrActors()];
        prevReachableNodes[a] = new bool [g->nrActors()];

        curReachableNodesCh[a] = new bool* [g->nrActors()];
        prevReachableNodesCh[a] = new bool* [g->nrActors()];

        for (uint b = 0; b < g->nrActors(); b++)
        {
            curReachableNodesCh[a][b] = new bool [g->nrChannels()];
            prevReachableNodesCh[a][b] = new bool [g->nrChannels()];
        }
    }
}

/**
 * initDependencies ()
 * The function initializes the dependency stack and the last dependency
 * node pointers.
 */
static inline
void initDependencies()
{
    for (uint a = 0; a < g->nrActors(); a++)
    {
        for (uint b = 0; b < g->nrActors(); b++)
        {
            if (a == b)
            {
                curReachableNodes[a][b] = true;
                prevReachableNodes[a][b] = true;
            }
            else
            {
                curReachableNodes[a][b] = false;
                prevReachableNodes[a][b] = false;
            }
        }
    }

    for (uint a = 0; a < g->nrActors(); a++)
    {
        for (uint b = 0; b < g->nrActors(); b++)
        {
            for (uint c = 0; c < g->nrChannels(); c++)
            {
                curReachableNodesCh[a][b][c] = false;
                prevReachableNodesCh[a][b][c] = false;
            }
        }
    }
}

/**
 * createDependencyNode ()
 * Create a new dependency node for the actor a and add the dependency
 * to the list 'curDependencyNode'.
 */
static inline
void createDependencyNode(int a)
{
    for (uint b = 0; b < g->nrActors(); b++)
        curReachableNodes[b][a] = false;
    
    for (uint b = 0; b < g->nrActors(); b++)
        for (uint c = 0; c < g->nrChannels(); c++)
            curReachableNodesCh[b][a][c] = false;
}

/**
 * addDependencyEdgeForActor ()
 * Add a dependency edge to the abstract depndency graph which expresses a
 * dependency of actor a to the previous firing of actor a.
 */
static inline
void addDependencyEdgeForActor(int a)
{
    for (uint b = 0; b < g->nrActors(); b++)
        curReachableNodes[b][a] = 
                        curReachableNodes[b][a]
                            || prevReachableNodes[b][a];
    
    for (uint b = 0; b < g->nrActors(); b++)
        for (uint c = 0; c < g->nrChannels(); c++)
            curReachableNodesCh[b][a][c] = 
                        curReachableNodesCh[b][a][c] 
                            || prevReachableNodesCh[b][a][c];
}

/**
 * addDependencyEdgeForChTokens ()
 * Add a dependency edge to the abstract dependency graph which expresses a
 * dependency of actor s to tokens produced on channel c by the previous
 * firing of actor a.
 */
static inline
void addDependencyEdgeForChTokens(int a, int c, int s)
{
    for (uint b = 0; b < g->nrActors(); b++)
        curReachableNodes[b][s] = 
                        curReachableNodes[b][s]
                            || prevReachableNodes[b][a];
    
    for (uint b = 0; b <g->nrActors(); b++)
        for (uint c = 0; c < g->nrChannels(); c++)
            curReachableNodesCh[b][s][c] = 
                        curReachableNodesCh[b][s][c] 
                            || prevReachableNodesCh[b][a][c];
}

/******************************************************************************
 * SDF
 *****************************************************************************/      
/**  
 * outputIntervalCycle ()  
 * The function calculates the output interval of the states on the cycle. Its  
 * value is equal to the average time between two firings in the cycle.  
 */  
static inline  
TDtime outputIntervalCycle(const StackPos cyclePos)  
{  
	int nr_fire = 0;  
	TDtime time = 0;  

	// Check all state from stack till cycle complete  
	for (StackPos ptr = stackPtr -1; ptr >= cyclePos; ptr--)  
	{  
	    // Output actor fired?  
        if (stack[ptr].act_clk[outputActor->getId()][0] > 0)  
            nr_fire++;  

	    // Time between previous state  
	    time += stack[ptr].glb_clk;  
	}  

	return time/(TDtime)(nr_fire);  
}  
	    
/**  
 * storeState ()  
 * The function stores the given state at the stack and insert an entry  
 * into the hash table. Cycle detection is performed, as well as a  
 * check to verify wether a cycle is valid or not.  
 *  
 * Return values:  
 *      -1      - added state to stack  
 *      >= 0    - output interval of detected cycle  
 */  
static inline  
TDtime storeState(State &sdfState)  
{  
    // Calculate the key in the hash table  
    HashKey key = hash(sdfState);  

    // Check occurance in hash table  
    StackPos stackPos = findValueInHashTable(key, sdfState);  
    if (stackPos == NO_SLOT_IN_HASH)  
    {  
        // State not in hash table (i.e. state not seen before)  
        // Insert state in hash table  
        insertKeyHashTable(key, stackPtr);  

        // Insert state in stack  
        pushStack(sdfState);  
    }  
    else  
    {  
        // State has been visited before (i.e. found cycle)  
        return outputIntervalCycle(stackPos);  
    }  

    return -1;  
}  

#define GLB_CLK             sdfState.glb_clk  
#define ACT_CLK(a,t)        sdfState.act_clk[a][t]  
#define CH(c)               sdfState.ch[c]  

#define FIRE_ACT(a,t)       ACT_CLK(a,t)++;  
#define FIRE_ACT_END(a)     ACT_CLK(a,0)--;  
#define CH_TOKENS(c,n)      (CH(c) >= n)  
#define CONSUME(c,n)        CH(c) = CH(c) - n;
#define PRODUCE(c,n)        CH(c) = CH(c) + n;  
#define ACT_END_FIRE(a)     (ACT_CLK(a,0) != 0)  
#define ADVANCE_CLK         GLB_CLK = GLB_CLK + 1;  
#define NEXT_ITER           GLB_CLK = 0;  
#define LOWER_CLK(a)        for (TCnt t = 0; t < SDF_MAX_TIME; t++) \
                                ACT_CLK(a,t) = ACT_CLK(a,t+1); \
                            ACT_CLK(a,SDF_MAX_TIME) = 0;

#define CH_TOKENS_PREV(c,n) (prevStateP.ch[c] >= n)

static State sdfState;  
static State prevState;  
static State prevStateP;

/**
 * analyzePeriodicPhase ()
 * Analyze the periodic phase of the schedule to find the abstract dependency
 * graph.
 */
static 
void analyzePeriodicPhase()
{
    bool outputActorFound = false;
    bool completedPeriod = false;
    State periodicState;
    int repCnt;  

    // Current sdf state is a periodic state
    createState(periodicState);
    copyState(periodicState, sdfState);

    // Initialize the dependencies
    initDependencies();

    // Start new iteration of the periodic phase
    NEXT_ITER;
    
    // Still need to complete the last firing of the output actor
    // before period really ends
    repCnt = -1;

    // Complete the remaining actor firings
    for (SDFactorsIter iter = g->actorsBegin();
            iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;

        if (!outputActorFound && a->getId() == outputActor->getId())
            outputActorFound = true;

        while (outputActorFound && ACT_END_FIRE(a->getId()))  
        {
            if (a->getId() == outputActor->getId())
            {
                repCnt++;
                if (repCnt == outputActorRepCnt)
                {
                    NEXT_ITER;
                    repCnt = 0;
                }
            }

            for (SDFportsIter pIter = a->portsBegin();
                    pIter != a->portsEnd(); pIter++)
            {
                SDFport *p = *pIter;
                SDFchannel *ch = p->getChannel();

                if (p->getType() == SDFport::Out)
                {
                    PRODUCE(ch->getId(), p->getRate());
                }
            }
            FIRE_ACT_END(a->getId());
        }
    }

    // Fire the actors
    while (true)
    {
        // Update previous actor dependencies (abstract dependency graph)
        for (uint a = 0; a < g->nrActors(); a++)
            for (uint b = 0; b < g->nrActors(); b++)
                prevReachableNodes[a][b] = curReachableNodes[a][b];
        
        for (uint a = 0; a < g->nrActors(); a++)
            for (uint b = 0; b < g->nrActors(); b++)
                for (uint c = 0; c < g->nrChannels(); c++)
                prevReachableNodesCh[a][b][c] = curReachableNodesCh[a][b][c];

        // Start actor firings
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)*iter;

            while (true)
            {
                bool fire = true;

                // Check all input ports for tokens
                for (SDFportsIter pIter = a->portsBegin();
                        pIter != a->portsEnd(); pIter++)
                {
                    SDFport *p = *pIter;
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::In)
                    {
                        if (!CH_TOKENS(ch->getId(), p->getRate()))
                        {
                            fire = false;
                            break;
                        }
                    }
                }

                // Ready to fire?
                if (fire)
                {
                    // Which dependencies were resolved in previous time step?
                    // Check tokens on each channel
                    for (SDFportsIter pIter = a->portsBegin();
                            pIter != a->portsEnd(); pIter++)
                    {
                        SDFport *p = *pIter;
                        SDFchannel *ch = p->getChannel();
                        SDFactor *b = ch->oppositePort(p)->getActor();

                        if (b->getId() == a->getId())
                        {
                            // Previous firing of actor not finished?
                            if (!CH_TOKENS_PREV(ch->getId(), p->getRate()))
                            {
                                addDependencyEdgeForActor(a->getId());
                            }
                        }
                        else if (p->getType() == SDFport::In)
                        {
                            if (!CH_TOKENS_PREV(ch->getId(), p->getRate()))
                            {
                                addDependencyEdgeForChTokens(b->getId(),
                                                ch->getId(), a->getId());
                            }
                        }
                    }

                    // Consume tokens on each input and space on each output
                    for (SDFportsIter pIter = a->portsBegin();
                            pIter != a->portsEnd(); pIter++)
                    {
                        SDFport *p = *pIter;
                        SDFchannel *ch = p->getChannel();

                        if (p->getType() == SDFport::In)
                        {
                            CONSUME(ch->getId(), p->getRate());
                        }
                    }

                    // Fire the actor
                    FIRE_ACT(a->getId(), a->getExecutionTime());
                }
                else
                {
                    break;
                }
            }
        }

        // Lower clocks
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            LOWER_CLK(a->getId());
        }

        // Advance the global clock
        ADVANCE_CLK;

        // Store state to check for progress
        copyState(prevStateP, sdfState);
        
        // Finish actor firings
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
    
            while (ACT_END_FIRE(a->getId()))  
            {
                if (outputActor->getId() == a->getId())
                {
                    repCnt++;
                    if (repCnt == outputActorRepCnt)  
                    {  
                        // Add state to hash of visited states  
                        if (equalStates(sdfState, periodicState))
                        {
                            completedPeriod = true;
                            break;
                        }
	                    NEXT_ITER;
                        repCnt = 0;  
                    }  
                }

                for (SDFportsIter pIter = a->portsBegin();
                        pIter != a->portsEnd(); pIter++)
                {
                    SDFport *p = *pIter;
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::Out)
                    {
                        PRODUCE(ch->getId(), p->getRate());
                    }
                }

                FIRE_ACT_END(a->getId());
            }
            
            if (completedPeriod)
                break;  
        }

        if (completedPeriod)
            break;
    }

    // Print the abstract dependency graph
    //for (uint a = 0; a < g->nrActors(); a++)
    //{
    //    cerr << a <<": " << endl;
    //    for (uint b = 0; b < g->nrActors(); b++)
    //    {
    //        if (curReachableNodes[a][b])
    //            cerr << b << ", ";
    //    }
    //    cerr << endl;
    //}
}

/**  
 * execSDFgraph()  
 * Execute the SDF graph till a deadlock is found or a recurrent state.  
 * The output interval (i.e. inverse of throughput) is returned.  
 */  
static
TDtime execSDFgraph()  
{  
    TDtime output_interval;  
    int repCnt = 0;  

    createState(sdfState);
    createState(prevState);  
    createState(prevStateP);  

    // Create initial state and add it to stack
    clearState(sdfState);  
    copyState(prevState, sdfState);  
    prevState.glb_clk = TDTIME_MAX;  

    // Initial tokens and space
    for (SDFchannelsIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *ch = *iter;

        CH(ch->getId()) = ch->getInitialTokens();
    }

    // Fire the actors  
    while (true)  
    {  
        // Store state to find actor activity in periodic phase
        copyState(prevStateP, sdfState);

        // Finish actor firings  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
    
            while (ACT_END_FIRE(a->getId()))  
            {
                if (outputActor->getId() == a->getId())
                {
                    repCnt++;
                    if (repCnt == outputActorRepCnt)  
                    {  
                        // Add state to hash of visited states  
                        output_interval = storeState(sdfState);  
                        if (output_interval != -1)
                        {
                            analyzePeriodicPhase(); 
                            return output_interval;
                        }
	                    NEXT_ITER;  
                        repCnt = 0;  
                    }  
                }

                for (SDFportsIter pIter = a->portsBegin();
                        pIter != a->portsEnd(); pIter++)
                {
                    SDFport *p = *pIter;
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::Out)
                    {
                        PRODUCE(ch->getId(), p->getRate());
                    }
                }

                FIRE_ACT_END(a->getId());
            }  
        }

        // Start actor firings  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)*iter;

            while (true)
            {
                bool fire = true;

                // Check all input ports for tokens
                for (SDFportsIter pIter = a->portsBegin();
                        pIter != a->portsEnd(); pIter++)
                {
                    SDFport *p = *pIter;
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::In)
                    {
                        if (!CH_TOKENS(ch->getId(), p->getRate()))
                        {
                            fire = false;
                            break;
                        }
                    }
                }

                // Ready to fire?
                if (!fire)
                    break;

                // Consume tokens on each input and space on each output
                for (SDFportsIter pIter = a->portsBegin();
                        pIter != a->portsEnd(); pIter++)
                {
                    SDFport *p = *pIter;
                    SDFchannel *ch = p->getChannel();

                    if (p->getType() == SDFport::In)
                    {
                        CONSUME(ch->getId(), p->getRate());
                    }
                }

                // Fire the actor  
                FIRE_ACT(a->getId(), a->getExecutionTime());
            }
        }
        
        // Lower clocks  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            LOWER_CLK(a->getId());
        }

	    // Check for progress (i.e. no deadlock)  
	    if (equalStates(prevState, sdfState))
        {
            return TDTIME_MAX;
        }
        
        // Advance the global clock  
        ADVANCE_CLK;  

	    // Store state to check for progress  
	    copyState(prevState, sdfState);  
    }
    
    destroyState(sdfState);
    destroyState(prevState);
    destroyState(prevStateP);
    
    return TDTIME_MAX;
}  

/**
 * stateSpaceAbstractDepGraph ()
 * Compute the throughput and abstract dependency graph of an SDFG for
 * unconstrained buffer sizes and using auto-concurrency using a state-space
 * traversal.
 *
 * The abstract dependency graph is stored in the matrices 'nodes' (2D) and
 * 'edges' (3D). The value stored at 'nodes[a][b]' indicates whether there is at
 * least one edge which goes directlt from node a to node b. The value stored at
 * 'edges[a][b][c]' indicates wether the channel c in the SDFG has a depedency
 * edge in the abstract dependency graph from node a to node b.
 */
extern
double stateSpaceAbstractDepGraph(TimedSDFgraph *gr, bool ***nodes, 
        bool ****edges, unsigned long long stackSz, unsigned long long hashSz)
{
    RepetitionVector repVec;
    double thr;
    
    // Copy arguments to globals
    g = gr;
	STACK_SIZE = stackSz;
    HASH_TABLE_SIZE = hashSz;

    // Check that the graph g is a strongly connected graph
    if (!isStronglyConnectedGraph(g))
        throw CException("Graph is not strongly connected.");
    
    // Find the output actor and its repetition count
    repVec = computeRepetitionVector(g);
    outputActor = findOutputActor(g, repVec);
    outputActorRepCnt = repVec[outputActor->getId()];

    // Find largest execution time of all actors
    SDF_MAX_TIME = getMaxExecTime(g);

    // Create hash and stack
    createStack(); clearStack();
	createHashTable(); clearHashTable();

    // Allocate memory space for the abstract dependency graph
    createDependencies();
    *nodes = curReachableNodes;
    *edges = curReachableNodesCh;

    // Find the maximal throughput
    thr = execSDFgraph();
    if (thr != TDTIME_MAX)
        thr = (double)(1.0)/thr;
    else
        thr = 0;

	// Cleanup
	destroyStack();
	destroyHashTable();
    
    return thr;
}
