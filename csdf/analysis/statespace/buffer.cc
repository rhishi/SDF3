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
 * $Id: buffer.cc,v 1.2 2007/11/02 09:34:46 sander Exp $
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

#include "statespace.h"
using namespace CSDF;

typedef unsigned short TCnt;

#define TBUFSIZE_MAX    ULONG_MAX

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

/**
 * Pointer to CSDF graph which is analyzed
 */
static TimedCSDFgraph *g;

/**
 * Globals
 * These are initialized by calling the function initGlobals.
 */
static TBufSize *minSz;
static TBufSize *minSzStep;
static TBufSize lbDistributionSz;
static TDtime maxThroughput;

static CSDFtime maxExecTimeActor;
static CSDFactor *outputActor;
static TCnt outputActorRepCnt;

static
void computeMinimalChannelSzStep(TimedCSDFgraph *g)
{
    minSzStep = new TBufSize [g->nrChannels()];
    
    for (CSDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        uint minStepSz;
        CSDFchannel *ch = *iter;
        CSDFport *srcPort = ch->getSrcPort();
        CSDFport *dstPort = ch->getDstPort();
        
        // Initialize minStepSz with a "randomly" selected rate from all
        // possible rates
        minStepSz = srcPort->getRate().getRate(0);
        
        // Step size is equal to the gcd of all producation and consumption
        // rates that are possible
        for (uint i = 0; i < srcPort->getRate().lengthRatesVector(); i++)
            minStepSz = gcd(minStepSz, srcPort->getRate().getRate(i));
        for (uint i = 0; i < dstPort->getRate().lengthRatesVector(); i++)
            minStepSz = gcd(minStepSz, dstPort->getRate().getRate(i));

        minSzStep[ch->getId()] = minStepSz;    
    }
}

static
void computeMinimalChannelSz(TimedCSDFgraph *g)
{
    minSz =  new TBufSize [g->nrChannels()];    

    for (CSDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        CSDFchannel *ch = *iter;
        CSDFport *srcPort = ch->getSrcPort();
        CSDFport *dstPort = ch->getDstPort();
        uint ratePeriod = gcd(srcPort->getRate().lengthRatesVector(),
                                dstPort->getRate().lengthRatesVector());
        
        // Initialize lower bound to maximal size
        minSz[ch->getId()] = UINT_MAX;
        
        // Iterate over all production/consumption rate combinations to find
        // lower bound as given for SDFG case.
        for (uint i = 0; i < ratePeriod; i++)
        {
            uint p = srcPort->getRate().getRate(i);
            uint c = dstPort->getRate().getRate(i);
            uint t = ch->getInitialTokens();
            uint lb;


            // Lower-bound of a self-edge is rate at which data is produced and
            // consumed and the number of initial tokens present
            if (ch->getSrcActor()->getId() == ch->getDstActor()->getId())
            {
                lb = p + (c > t ? c : t);
            }
            else
            {
                lb = p+c-gcd(p,c)+t%gcd(p,c);
                lb = (lb > t ? lb : t);
            }

            if (minSz[ch->getId()] > lb)
                minSz[ch->getId()] = lb;
        }
        
    }
}

static
void computeLbDistributionSz(TimedCSDFgraph *g)
{
    lbDistributionSz = 0;
    
    for (uint c = 0; c < g->nrChannels(); c++)
        lbDistributionSz += minSz[c];
}

static
void findOutputActor(TimedCSDFgraph *g)
{
    CSDFgraph::RepetitionVector repVec = g->getRepetitionVector();
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
    
    outputActor = a;
    outputActorRepCnt = repVec[a->getId()];
}

static
void findMaxExecTime(TimedCSDFgraph *g)
{
    CSDFtime max = 0;
    
    for (CSDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        TimedCSDFactor *a = (TimedCSDFactor*)(*iter);
        CCSDFtime &execTime = a->getExecutionTime();
        
        for (uint i = 0; i < execTime.size(); i++)
            if (execTime.value(i) > max)
                max = execTime.value(i);
    }
    
    maxExecTimeActor = max;
}

static
void findMaxThroughput(TimedCSDFgraph *g)
{
    maxThroughput = stateSpaceThroughputAnalysis(g);
}

static
void initGlobals(TimedCSDFgraph *g)
{
    computeMinimalChannelSzStep(g);
    computeMinimalChannelSz(g);
    computeLbDistributionSz(g);
    findOutputActor(g);
    findMaxExecTime(g);
    findMaxThroughput(g);
}

/******************************************************************************   * State  
 *****************************************************************************/
typedef struct _State  
{  
    TCnt **act_clk;
    unsigned long glb_clk;  
    TBufSize *ch; 
    TBufSize *sp; 
    TCnt **ratePos;
    TCnt *execTimePos;
} State;  

/**  
 * clearState ()  
 * The function sets the state to zero.  
 */  
static
void clearState(State &s)  
{
    for (uint i = 0; i < g->nrChannels(); i++)
    {
        s.ch[i] = 0;
        s.sp[i] = 0;
    }
    
    s.glb_clk = 0;

    for (uint i = 0; i < g->nrActors(); i++)
    {
        for (TCnt t = 0; t < maxExecTimeActor+1; t++)
            s.act_clk[i][t] = 0;
        
        s.execTimePos[i] = 0;
    }
    
    for (uint i = 0; i < g->nrChannels(); i++)
    {
        s.ratePos[i][0] = 0;
        s.ratePos[i][1] = 0;
    }
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
        if (s1.ch[i] != s2.ch[i] || s1.sp[i] != s2.sp[i])
            return false;

    for (uint i = 0; i < g->nrChannels(); i++)
        if (s1.ratePos[i][0] != s2.ratePos[i][0]
                || s1.ratePos[i][1] != s2.ratePos[i][1])
            return false;
    
    for (uint i = 0; i < g->nrActors(); i++)
    {
        for (uint t = 0; t < maxExecTimeActor+1; t++)
            if (s1.act_clk[i][t] != s2.act_clk[i][t])
                return false;
                
        if (s1.execTimePos[i] != s2.execTimePos[i])
            return false;
    }

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
    {
        to.ch[i] = from.ch[i];
        to.sp[i] = from.sp[i];
    }
    
    for (uint i = 0; i < g->nrActors(); i++)
    {
        for (uint t = 0; t < maxExecTimeActor+1; t++)
            to.act_clk[i][t] = from.act_clk[i][t];
            
        to.execTimePos[i] = from.execTimePos[i];
    }
    
    for (uint i = 0; i < g->nrChannels(); i++)
    {
        to.ratePos[i][0] = from.ratePos[i][0];
        to.ratePos[i][1] = from.ratePos[i][1];
    }
}

/**  
 * createState ()  
 * The function allocates memory for a new state.
 */  
static
void createState(State &s)  
{
    s.ch = new TBufSize [g->nrChannels()];
    s.sp = new TBufSize [g->nrChannels()];
    
    s.act_clk = new TCnt* [g->nrActors()];
    for (uint i = 0; i < g->nrActors(); i++)
        s.act_clk[i] = new TCnt [maxExecTimeActor+1];

    s.execTimePos = new TCnt [g->nrActors()];

    s.ratePos = new TCnt* [g->nrChannels()];
    for (uint i = 0; i < g->nrChannels(); i++)
        s.ratePos[i] = new TCnt [2];
}  

/**  
 * destroyState ()  
 * The function deallocates all data structures of the state
 */  
static
void destroyState(State &s)  
{
    delete [] s.ch;
    delete [] s.sp;
    
    for (uint i = 0; i < g->nrActors(); i++)
        delete [] s.act_clk[i];
    delete [] s.act_clk;

    delete [] s.execTimePos;
    
    for (uint i = 0; i < g->nrChannels(); i++)
        delete [] s.ratePos[i];
    delete [] s.ratePos;
} 

#if 0
/**
 * printState ()
 * The function outputs the state to the output stream.
 */
static
void printState(ostream &out, const State &s)  
{
    out << "((";
    for (uint i = 0; i < g->nrChannels(); i++)
    {
        out << s.ch[i];
        if (i != g->nrChannels() -1 )
            out << ", ";
    }
    out << "), (";
    for (uint i = 0; i < g->nrChannels(); i++)
    {
        out << s.sp[i];
        if (i != g->nrChannels() -1 )
            out << ", ";
    }
    out << "), " << s.glb_clk << ", (";
    for (uint i = 0; i < g->nrActors(); i++)
    {
        out << "{";
        for (TCnt t = 0; t < maxExecTimeActor+1; t++)
        {
            out << s.act_clk[i][t];
            if (t != maxExecTimeActor)
                out << ", ";
        }
        out << "}";
        if (i != g->nrActors() - 1)
            out << ", ";
    }
    out << "))" << endl;
}  
#endif 

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
        cerr << "[INFO] resizing stack." << endl;
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
        for (TCnt t = 0; t < maxExecTimeActor+1; t++)
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
 * dfsVisitDependencies ()
 * The function performs a DFS on the abstract dependency graph from a node a to
 * find all cycles of which a is part. Channels on a cycle are when needed
 * marked to have a storage dependency.
 */
static
void dfsVisitDependencies(uint a, int *color, int *pi,
        bool **abstractDepGraph, bool *dep)
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
                    for (CSDFchannelsIter iter = g->channelsBegin();
                            iter != g->channelsEnd(); iter++)
                    {
                        CSDFchannel *ch = *iter;
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
static
void findStorageDependencies(bool **abstractDepGraph, bool *dep)
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
/**  
 * computeThroughput ()  
 * The function calculates the throughput of the states on the cycle. Its  
 * value is equal to the average number of firings of an actor per time unit.
 */  
static inline  
TDtime computeThroughput(const StackPos cyclePos)  
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

	return (TDtime)(nr_fire)/(time);  
}  
	    
/**  
 * storeState ()  
 * The function stores the given state at the stack and insert an entry  
 * into the hash table. Cycle detection is performed, as well as a  
 * check to verify wether a cycle is valid or not.  
 *  
 * Return values:  
 *      -1      - state did not exist on the stack; the state is added
 *      >= 0    - state did exist on the stack; position of state on stack is
 *                returned
 */  
static inline  
StackPos storeState(State &s)  
{  
    // Calculate the key in the hash table  
    HashKey key = hash(s);  

    // Check occurance in hash table  
    StackPos stackPos = findValueInHashTable(key, s);  
    if (stackPos == NO_SLOT_IN_HASH)  
    {  
        // State not in hash table (i.e. state not seen before)  
        // Insert state in hash table  
        insertKeyHashTable(key, stackPtr);

        // Insert state in stack  
        pushStack(s);  
    }  
    else  
    {  
        // State has been visited before (i.e. found cycle)
        return stackPos;
    }  

    return -1;  
}  

#define GLB_CLK             currentState.glb_clk  
#define ACT_CLK(a,t)        currentState.act_clk[a][t]  
#define CH(c)               currentState.ch[c]  
#define SP(c)               currentState.sp[c]  

#define FIRE_ACT(a,t)       ACT_CLK(a,t)++;  
#define FIRE_ACT_END(a)     ACT_CLK(a,0)--;  
#define CH_TOKENS(c,n)      (CH(c) >= n)  
#define CH_SPACE(c,n)       (SP(c) >= n)
#define CONSUME(c,n)        CH(c) = CH(c) - n;
#define PRODUCE(c,n)        CH(c) = CH(c) + n;
#define CONSUME_SP(c,n)     SP(c) = SP(c) - n;
#define PRODUCE_SP(c,n)     SP(c) = SP(c) + n;
#define ACT_END_FIRE(a)     (ACT_CLK(a,0) != 0)  
#define ADVANCE_CLK         GLB_CLK = GLB_CLK + 1;  
#define NEXT_ITER           GLB_CLK = 0;  
#define LOWER_CLK(a)        for (TCnt t = 0; t < maxExecTimeActor; t++) \
                                ACT_CLK(a,t) = ACT_CLK(a,t+1); \
                            ACT_CLK(a,maxExecTimeActor) = 0;

#define CH_TOKENS_PREV(c,n) (previousState.ch[c] >= n)
#define CH_SPACE_PREV(c,n)  (previousState.sp[c] >= n)

#define ADVANCE_SRC_RATE(c,s)  currentState.ratePos[c][0] = s;
#define ADVANCE_DST_RATE(c,s)  currentState.ratePos[c][1] = s;

#define ADVANCE_EXECTIME(a,s)  currentState.execTimePos[a] = s;

static State currentState;  
static State previousState;  

/**
 * analyzePeriodicPhase ()
 * Analyze the periodic phase of the schedule to find all blocked channels. This
 * is done using the abstract dependency graph.
 */
static 
void analyzePeriodicPhase(const TBufSize *sp, bool *dep)
{
    bool outputActorFound = false;
    bool **abstractDepGraph;
    State periodicState;
    int repCnt;  

    // Current sdf state is a periodic state
    createState(periodicState);
    copyState(periodicState, currentState);

    // Abstract dependency graph
    abstractDepGraph = new bool* [g->nrActors()];
    for (uint i = 0; i < g->nrActors(); i++)
    {
        abstractDepGraph[i] = new bool [g->nrActors()];
        for (uint j = 0; j < g->nrActors(); j++)
            abstractDepGraph[i][j] = false;
    }

    // Start new iteration of the periodic phase
    NEXT_ITER;
    
    // Still need to complete the last firing of the output actor
    // before period really ends
    repCnt = -1;

    // Complete the remaining actor firings
    for (CSDFactorsIter iter = g->actorsBegin();
            iter != g->actorsEnd(); iter++)
    {
        CSDFactor *a = *iter;

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

            for (CSDFportsIter pIter = a->portsBegin();
                    pIter != a->portsEnd(); pIter++)
            {
                CSDFport *p = *pIter;
                CSDFchannel *ch = p->getChannel();

                if (p->getPortType() == CSDFport::Out)
                {
                    PRODUCE(ch->getId(), p->getRate().advanceState());
                    ADVANCE_DST_RATE(ch->getId(),
                                        p->getRate().getCurrentState());
                }
                else
                {
                    PRODUCE_SP(ch->getId(), p->getRate().getCurrentRate());
                }
            }
            FIRE_ACT_END(a->getId());
        }
    }

    // Fire the actors
    while (true)
    {
        // Start actor firings
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedCSDFactor *a = (TimedCSDFactor*)*iter;
            bool fire = true;

            while (fire)
            {

                // Check all input ports for tokens and output ports
                // for space
                for (CSDFportsIter pIter = a->portsBegin();
                        pIter != a->portsEnd(); pIter++)
                {
                    CSDFport *p = *pIter;
                    CSDFchannel *ch = p->getChannel();

                    if (p->getPortType() == CSDFport::Out)
                    {
                        if (!CH_SPACE(ch->getId(),
                                            p->getRate().getCurrentRate()))
                        {
                            fire = false;
                            break;
                        }    
                    }
                    else
                    {
                        if (!CH_TOKENS(ch->getId(),
                                            p->getRate().getCurrentRate()))
                        {
                            fire = false;
                            break;
                        }
                    }
                }

                // Ready to fire?
                if (fire)
                {
                    // Which dependencies have been resolved in current state
                    // (time step) that the actor is now ready to fire and was
                    // not able to fire in the previous state?
                    
                    // Check tokens (space) on every input (output) channel
                    for (CSDFportsIter pIter = a->portsBegin();
                            pIter != a->portsEnd(); pIter++)
                    {
                        CSDFport *p = *pIter;
                        CSDFchannel *ch = p->getChannel();
                        CId srcId = ch->getSrcActor()->getId();
                        CId dstId = ch->getDstActor()->getId();
                        
                        if (p->getPortType() == CSDFport::In)
                        {
                            if (!CH_TOKENS_PREV(ch->getId(),
                                    p->getRate().getCurrentRate()))
                            {
                                abstractDepGraph[dstId][srcId] = true;
                            }
                        }
                        else
                        {
                            if (!CH_SPACE_PREV(ch->getId(),
                                    p->getRate().getCurrentRate()))
                            {
                                abstractDepGraph[srcId][dstId] = true;
                            }
                        }
                    }

                    // Consume tokens on each input and space on each output
                    for (CSDFportsIter pIter = a->portsBegin();
                            pIter != a->portsEnd(); pIter++)
                    {
                        CSDFport *p = *pIter;
                        CSDFchannel *ch = p->getChannel();

                        if (p->getPortType() == CSDFport::In)
                        {
                            CONSUME(ch->getId(), p->getRate().advanceState());
                            ADVANCE_SRC_RATE(ch->getId(),
                                        p->getRate().getCurrentState());
                        }
                        else
                        {
                            CONSUME_SP(ch->getId(),
                                        p->getRate().getCurrentRate());
                        }
                    }

                    // Fire the actor
                    FIRE_ACT(a->getId(), a->getExecutionTime().value());
                    a->getExecutionTime().next();
                    ADVANCE_EXECTIME(a->getId(),
                                a->getExecutionTime().position());
                }
            }
        }

        // Maximal time progress
        TCnt timestep = maxExecTimeActor + 1;
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedCSDFactor *a = (TimedCSDFactor*)(*iter);

            for (TCnt t = 1; t < timestep; t++)
            {
                if (ACT_CLK(a->getId(), t) > 0)
                {
                    timestep = t;
                    break;
                }
            }
        }

        // Lower clocks
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedCSDFactor *a = (TimedCSDFactor*)(*iter);

            for (TCnt t = 0; t < maxExecTimeActor - timestep + 1; t++)
            {
                ACT_CLK(a->getId(),t) = ACT_CLK(a->getId(),t+timestep);
            }
            for (TCnt t = maxExecTimeActor - timestep + 1; 
                        t <= maxExecTimeActor; t++)
            {
                ACT_CLK(a->getId(),t) = 0;
            }
        }

        // Advance the global clock  
        GLB_CLK = GLB_CLK + timestep;  

        // Store partial state to check for progress
        for (uint i = 0; i < g->nrChannels(); i++)
        {
            previousState.ch[i] = currentState.ch[i];
            previousState.sp[i] = currentState.sp[i];
        }
                
        // Finish actor firings
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            CSDFactor *a = *iter;
    
            while (ACT_END_FIRE(a->getId()))  
            {
                if (outputActor->getId() == a->getId())
                {
                    repCnt++;
                    if (repCnt == outputActorRepCnt)  
                    {  
                        // Add state to hash of visited states  
                        if (equalStates(currentState, periodicState))
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
	                    NEXT_ITER;
                        repCnt = 0;  
                    }  
                }

                for (CSDFportsIter pIter = a->portsBegin();
                        pIter != a->portsEnd(); pIter++)
                {
                    CSDFport *p = *pIter;
                    CSDFchannel *ch = p->getChannel();

                    if (p->getPortType() == CSDFport::Out)
                    {
                        PRODUCE(ch->getId(), p->getRate().advanceState());
                        ADVANCE_DST_RATE(ch->getId(),
                                        p->getRate().getCurrentState());
                    }
                    else
                    {
                        PRODUCE_SP(ch->getId(), p->getRate().getCurrentRate());
                    }
                }

                FIRE_ACT_END(a->getId());
            }
        }
    }
}

/**
 * analyzeDeadlock ()
 * Analyze the deadlock in the schedule.
 */
static 
void analyzeDeadlock(const TBufSize *sp, bool *dep)
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
    for (CSDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        CSDFchannel *c = *iter;
        CSDFactor *src = c->getSrcActor();
        CSDFactor *dst = c->getDstActor();
        
        // Insufficient tokens to fire destination actor
        if (!CH_TOKENS(c->getId(), c->getDstPort()->getRate().getCurrentRate()))
        {
            abstractDepGraph[dst->getId()][src->getId()] = true;
        }

        // Insufficient space to fire source actor
        if (!CH_SPACE(c->getId(), c->getSrcPort()->getRate().getCurrentRate()))
        {
            abstractDepGraph[src->getId()][dst->getId()] = true;
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
 * execCSDFgraph()  
 * Execute the CSDF graph till a deadlock is found or a recurrent state.  
 * The throughput is returned.  
 */  
static
TDtime execCSDFgraph(const TBufSize *sp, bool *dep)  
{
    StackPos recurrentState;
    int repCnt = 0;  

    // Reset position of execution time and rate sequences
    for (CSDFchannelsIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        CSDFchannel *c = *iter;
        
        c->getSrcPort()->getRate().clearState();
        c->getDstPort()->getRate().clearState();
    }
    for (CSDFactorsIter iter = g->actorsBegin();
            iter != g->actorsEnd(); iter++)
    {
        TimedCSDFactor *a = (TimedCSDFactor*)*iter;
        
        a->getExecutionTime().reset();
    }

    createState(currentState);
    createState(previousState);  

    // Create initial state
    clearState(currentState);  

    // Initial tokens and space
    for (CSDFchannelsIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        CSDFchannel *ch = *iter;

        // Not enough space for initial tokens?
        if (sp[ch->getId()] < ch->getInitialTokens())
        {
            dep[ch->getId()] = true;
            return 0;
        }

        CH(ch->getId()) = ch->getInitialTokens();
        SP(ch->getId()) = sp[ch->getId()] - ch->getInitialTokens();
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
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            CSDFactor *a = *iter;

            while (ACT_END_FIRE(a->getId()))  
            {
                if (outputActor->getId() == a->getId())
                {
                    repCnt++;
                    if (repCnt == outputActorRepCnt)  
                    { 
                        // Add state to hash of visited states  
                        recurrentState = storeState(currentState);  
                        if (recurrentState != -1)
                        {
                            // Find storage dependencies in periodic phase
                            analyzePeriodicPhase(sp, dep);

                            return computeThroughput(recurrentState);
                        }
	                    NEXT_ITER;  
                        repCnt = 0;  
                    }  
                }

                for (CSDFportsIter pIter = a->portsBegin();
                        pIter != a->portsEnd(); pIter++)
                {
                    CSDFport *p = *pIter;
                    CSDFchannel *ch = p->getChannel();

                    if (p->getPortType() == CSDFport::Out)
                    {
                        PRODUCE(ch->getId(), p->getRate().advanceState());
                        ADVANCE_DST_RATE(ch->getId(),
                                        p->getRate().getCurrentState());
                    }
                    else
                    {
                        PRODUCE_SP(ch->getId(), p->getRate().getCurrentRate());
                    }
                }

                FIRE_ACT_END(a->getId());
            }  
        }

        // Start actor firings  
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedCSDFactor *a = (TimedCSDFactor*)*iter;
            bool fire = true;

            while (fire)
            {
                // Check all input ports for tokens and output ports
                // for space
                for (CSDFportsIter pIter = a->portsBegin();
                        pIter != a->portsEnd(); pIter++)
                {
                    CSDFport *p = *pIter;
                    CSDFchannel *ch = p->getChannel();

                    if (p->getPortType() == CSDFport::Out)
                    {
                        if (!CH_SPACE(ch->getId(),
                                    p->getRate().getCurrentRate()))
                        {
                            fire = false;
                            break;
                        }    
                    }
                    else
                    {
                        if (!CH_TOKENS(ch->getId(),
                                    p->getRate().getCurrentRate()))
                        {
                            fire = false;
                            break;
                        }
                    }
                }

                // Ready to fire?
                if (fire)
                {
                    // Consume tokens on each input and space on each output
                    for (CSDFportsIter pIter = a->portsBegin();
                            pIter != a->portsEnd(); pIter++)
                    {
                        CSDFport *p = *pIter;
                        CSDFchannel *ch = p->getChannel();

                        if (p->getPortType() == CSDFport::In)
                        {
                            CONSUME(ch->getId(), p->getRate().advanceState());
                            ADVANCE_SRC_RATE(ch->getId(),
                                            p->getRate().getCurrentState());
                        }
                        else
                        {
                            CONSUME_SP(ch->getId(),
                                            p->getRate().getCurrentRate());
                        }
                    }

                    // Fire the actor  
                    FIRE_ACT(a->getId(), a->getExecutionTime().value());
                    a->getExecutionTime().next();
                    ADVANCE_EXECTIME(a->getId(),
                                a->getExecutionTime().position());
                }
            }
        }

        // Maximal time progress
        TCnt timestep = maxExecTimeActor + 1;
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedCSDFactor *a = (TimedCSDFactor*)(*iter);

            for (TCnt t = 1; t < timestep; t++)
            {
                if (ACT_CLK(a->getId(), t) > 0)
                {
                    timestep = t;
                    break;
                }
            }
        }

	    // Check for progress (i.e. no deadlock) 
	    if (timestep == maxExecTimeActor+1)
        {
            // Find cause of deadlock
            analyzeDeadlock(sp, dep);
            return 0;
        }

        // Lower clocks
        for (CSDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedCSDFactor *a = (TimedCSDFactor*)(*iter);

            for (TCnt t = 0; t < maxExecTimeActor - timestep + 1; t++)
            {
                ACT_CLK(a->getId(),t) = ACT_CLK(a->getId(),t+timestep);
            }
            for (TCnt t = maxExecTimeActor - timestep + 1; 
                        t <= maxExecTimeActor; t++)
            {
                ACT_CLK(a->getId(),t) = 0;
            }
        }
        
        // Advance the global clock  
        GLB_CLK = GLB_CLK + timestep;  
    }
    
    destroyState(currentState);
    destroyState(previousState);
    
    return 0;
}  

/*******************************************************************************
 * Distributions
 ******************************************************************************/

/**
 * minStorageDistributions
 * List of all minimal storage distributions.
 */
static CSDF::DistributionSet *minStorageDistributions = NULL;

/**
 * newDistribution ()
 * Allocate memory for a new storage distribution.
 */
static
CSDF::Distribution *newDistribution()
{
    CSDF::Distribution *d;
    
    d = new CSDF::Distribution;
    d->sp = new TBufSize [g->nrChannels()];
    d->dep = new bool [g->nrChannels()];
    
    return d;
}

/**
 * deleteDistribution ()
 * Deallocate memory for a storage distribution.
 */
static
void deleteDistribution(CSDF::Distribution *d)
{
    delete [] d->sp;
    delete [] d->dep;
    delete d;
}

/**
 * execDistribution ()
 * Compute throughput and storage dependencies of the given storage
 * distribution.
 */
static
void execDistribution(CSDF::Distribution *d)
{
    // Clear stack and hash table
    clearStack();
    clearHashTable();
    
    // Initialize blocking channels
    for (uint c = 0; c < g->nrChannels(); c++)
        d->dep[c] = false;

    // Execute the CSDF graph to find its output interval
    d->thr = execCSDFgraph(d->sp, d->dep);

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
static
void minimizeStorageDistributionsSet(CSDF::DistributionSet *ds)
{
    CSDF::Distribution *d, *t;
    
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
            deleteDistribution(d);

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
                deleteDistribution(d);

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
 * addDistributionToChecklist ()
 * The function add the storage distribution 'd' to the list of
 * storage distributions which must still be checked. This list is
 * ordered wrt to the size of the storage distribution. A storage
 * distribution is only added if it is not already contained in the
 * list. When the distribution is added to the list, the function returns
 * 'true', else the function returns 'false'.
 */
static
bool addDistributionToChecklist(CSDF::Distribution *d)
{
    CSDF::DistributionSet *ds, *dsNew;
    CSDF::Distribution *di;
    bool equalDistr;
    
    // First distribution ever added?
    if (minStorageDistributions == NULL)
    {
        // Create new set of storage distributions
        dsNew = new CSDF::DistributionSet;
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
        dsNew = new CSDF::DistributionSet;
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
        dsNew = new CSDF::DistributionSet;
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
 * exploreDistribution ()
 * The function computes the throughput of the storage distribution
 * 'd' and adds new storage distributions to the list of distributions
 * which must be checked based on the storage dependencies found in d.
 * The function also updates the maximal throughput of the set of
 * storage distributions when needed.
 */
static
void exploreDistribution(CSDF::DistributionSet *ds, CSDF::Distribution *d)
{
    CSDF::Distribution *dNew;
    
    // Compute throughput and storage dependencies
    execDistribution(d);
    
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
            dNew = newDistribution();
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
            if (!addDistributionToChecklist(dNew))
            {
                // Distribution already in check list
                deleteDistribution(dNew);
            }
        }
    }
}

/**
 * exploreDistributionSet ()
 * Explore all distributions within the set and remove all non-minimal
 * distributions from it.
 */
static
void exploreDistributionSet(CSDF::DistributionSet *ds)
{
    CSDF::Distribution *d;

    // Explore all storage distributions contained in the set    
    d = ds->distributions;
    while (d != NULL)
    {
        // Explore distribution d
        exploreDistribution(ds, d);
        
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
static
void findMinimalStorageDistributions(const double thrBound)
{
    CSDF::Distribution *d, *t;
    CSDF::DistributionSet *ds, *dt;
    
    // Construct storage distribution with lower bound storage space
    d = newDistribution();
    d->thr = 0;
    d->sz = lbDistributionSz;
    for (uint c = 0; c < g->nrChannels(); c++)
        d->sp[c] = minSz[c];
    d->next = NULL;
    d->prev = NULL;
    
    // Add distribution to set of distributions which must be checked
    addDistributionToChecklist(d);
    
    // Check sets of storage distributions till no distributions left to check,
    // or throughput bound exceeded, or maximal throughput reached
    ds = minStorageDistributions;
    while (ds != NULL)
    {
        // Explore all distributions with size 'ds->sz'
        exploreDistributionSet(ds);

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
                deleteDistribution(d);
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
 * stateSpaceBufferAnalysis ()
 * Analyze the trade-offs between storage distributions and throughput (using
 * auto-concurrency). The search ends as soon as the throughput bound (thrBound)
 * is reached. To find the complete pareto-space, the throughput bound should
 * be set to DOUBLE_MAX.
 */
CSDF::DistributionSet *stateSpaceBufferAnalysis(TimedCSDFgraph *gr,
        const double thrBound, unsigned long long stackSz, 
        unsigned long long hashSz)
{
    // Copy arguments to globals
    g = gr;
	STACK_SIZE = stackSz;
    HASH_TABLE_SIZE = hashSz;

    // Initialize storage distribution bounds and other globals
    initGlobals(g);

    // Create hash and stack
    createStack(); clearStack();
	createHashTable(); clearHashTable();

    // Search the space
    findMinimalStorageDistributions(thrBound);
    
	// Cleanup
	destroyStack();
	destroyHashTable();

    return minStorageDistributions;
}
