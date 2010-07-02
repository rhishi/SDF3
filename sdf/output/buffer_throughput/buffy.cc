/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffy.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   Buffy model
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *
 * $Id: buffy.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "buffy.h"
#include "../../base/algo/repetition_vector.h"

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

static
void printMinimalChannelSzStep(TimedSDFgraph *g, ostream &out)
{
    bool first = true;

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

        if (first)
        {
            first = false;
            out << "{" << minStepSz;
        }
        else
        {
            out << ", " << minStepSz;
        }
    }
    out << "}";
}

static
void printMaximumChannelSz(TimedSDFgraph *g, ostream &out)
{
    bool first = true;

    RepetitionVector repVec = computeRepetitionVector(g);

    for (SDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        SDFchannel *ch = *iter;
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint t = ch->getInitialTokens();
        uint ub;
        
        ub = p * repVec[srcPort->getActor()->getId()] 
                + c * repVec[dstPort->getActor()->getId()]
                + t%gcd(p,c);
        ub = (ub > t ? ub : t);
        
        if (first)
        {
            first = false;
            out << "{" << ub;
        }
        else
        {
            out << ", " << ub;
        }
    }
    out << "}";
}

static
uint getUbDistributionSz(TimedSDFgraph *g)
{
    uint ub = 0;

    RepetitionVector repVec = computeRepetitionVector(g);

    for (SDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        SDFchannel *ch = *iter;
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint t = ch->getInitialTokens();
        uint ub1;

        ub1 = p * repVec[srcPort->getActor()->getId()] 
                + c * repVec[dstPort->getActor()->getId()]
                + t%gcd(p,c);
        ub = ub + (ub1 > t ? ub1 : t);
    }

    return ub;
}

static
void printMinimalChannelSz(TimedSDFgraph *g, ostream &out)
{
    bool first = true;

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

        if (first)
        {
            first = false;
            out << "{" << lb;
        }
        else
        {
            out << ", " << lb;
        }
    }
    out << "}";
}

static
uint getLbDistributionSz(TimedSDFgraph *g)
{
    uint lb = 0;
    
    for (SDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        SDFchannel *ch = *iter;
        SDFport *srcPort = ch->getSrcPort();
        SDFport *dstPort = ch->getDstPort();
        uint p = srcPort->getRate();
        uint c = dstPort->getRate();
        uint t = ch->getInitialTokens();
        
        lb += p+c-gcd(p,c)+t%gcd(p,c);
    }
    
    return lb;
}

static
unsigned long long getLbOutputInterval(TimedSDFgraph *g)
{
    unsigned long long min = UINT_MAX;
        
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*iter);
        min = (min < a->getExecutionTime() ? min : a->getExecutionTime());
    }

    return min;
}

static
void printDefinitions(TimedSDFgraph *g, ostream &out, 
        unsigned long long stackSz, unsigned long long hashSz, 
        unsigned long long depStackSz)
{
    out << "#define SDF_NUM_ACTORS          " << g->nrActors() << endl;
    out << "#define SDF_NUM_CHANNELS        " << g->nrChannels() << endl;
    out << "" << endl;
    out << "#define LB_DISTRIBUTION_SZ      " << getLbDistributionSz(g) << endl;
    out << "#define UB_DISTRIBUTION_SZ      " << getUbDistributionSz(g) << endl;
    out << "" << endl;
    out << "#define LB_OUTPUT_INTERVAL      " << getLbOutputInterval(g) << endl;
    out << "" << endl;
    out << "#define MIN_CHANNEL_SZ          ";

    printMinimalChannelSz(g, out);
    
    out << endl;
    out << "#define MAX_CHANNEL_SZ          ";

    printMaximumChannelSz(g, out);

    out << endl;
    out << "#define MIN_CHANNEL_SZ_STEP     ";
    
    printMinimalChannelSzStep(g, out);
    
    out << endl;
    out << "" << endl;
    out << "#define STACK_SIZE              " << stackSz << endl;
    out << "#define HASH_TABLE_SIZE         " << hashSz << endl;
    out << "#define DEPENDENCY_STACK_SIZE   " << depStackSz << endl;
    out << "" << endl;
    out << "typedef short TBufSize;" << endl;
    out << "typedef double TTime;" << endl;
    out << endl;
    out << "#define TTIME_MAX INT_MAX" << endl;
    out << endl;
    out << "TBufSize minSz[] = MIN_CHANNEL_SZ;" << endl;
    out << "TBufSize maxSz[] = MAX_CHANNEL_SZ;" << endl;
    out << "TBufSize minSzStep[] = MIN_CHANNEL_SZ_STEP;" << endl;
    out << "" << endl;
    out << "TBufSize maxDistributionSz = UB_DISTRIBUTION_SZ;" << endl;
    out << "TTime lbOutputInterval = LB_OUTPUT_INTERVAL;" << endl;
    out << "" << endl;
}

static
void printMiscFunctions(ostream &out)
{
    out << "#include <math.h>" << endl;
    out << "#include <iostream>" << endl;
    out << "#include <assert.h>" << endl;
    out << "" << endl;
    out << "using std::ostream;" << endl;
    out << "using std::cout;" << endl;
    out << "using std::cerr;" << endl;
    out << "using std::endl;" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * exit ()" << endl;
    out << " * Exit the program with an error message." << endl;
    out << " */" << endl;
    out << "void exit(const char *msg, const int errorno)" << endl;
    out << "{" << endl;
    out << "    cerr << msg << endl;" << endl;
    out << "    exit(errorno);" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printState(ostream &out)
{
	out << "/******************************************************************************" << endl;
	out << " * State" << endl;
	out << " *****************************************************************************/" << endl;
	out << "" << endl;
	out << "typedef struct _State" << endl;
	out << "{" << endl;
	out << "    TTime act_clk[SDF_NUM_ACTORS];" << endl;
	out << "    TTime glb_clk;" << endl;
	out << "    TBufSize ch[SDF_NUM_CHANNELS];" << endl;
	out << "} State;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * printState ()" << endl;
	out << " * The function writes the state to the supplied stream." << endl;
	out << " */" << endl;
	out << "void printState(ostream &out, const State &s)" << endl;
	out << "{" << endl;
	out << "    out << \"State:\" << endl;" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < SDF_NUM_ACTORS; i++)" << endl;
	out << "        out << \"act_clk[\" << i << \"] = \" << s.act_clk[i] << endl;" << endl;
	out << "" << endl;
	out << "    out << \"glb_clk = \" << s.glb_clk << endl;" << endl;
	out << "" << endl;
	out << "    for (int i = 0; i < SDF_NUM_CHANNELS; i++)" << endl;
	out << "        out << \"ch[\" << i << \"] = \" << s.ch[i] << endl;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * clearState ()" << endl;
	out << " * The function sets the state to zero." << endl;
	out << " */" << endl;
	out << "void clearState(State &s)" << endl;
	out << "{" << endl;
	out << "    char *s_ptr = (char*)&s;" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < sizeof(State); i++)" << endl;
	out << "        s_ptr[i] = 0;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * equalStates ()" << endl;
	out << " * The function compares to states and returns true if they are equal." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "bool equalStates(const State &s1, const State &s2)" << endl;
	out << "{" << endl;
	out << "    int *s1_ptr = (int*)&s1;" << endl;
	out << "    int *s2_ptr = (int*)&s2;" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < sizeof(State) >> 2; i++)" << endl;
	out << "        if (s1_ptr[i] != s2_ptr[i])" << endl;
	out << "            return false;" << endl;
	out << "    " << endl;
	out << "    return true;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * copyState ()" << endl;
	out << " * The function copies the state." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void copyState(State &to, const State &from)" << endl;
	out << "{" << endl;
	out << "    memcpy(&to, &from, sizeof(State));" << endl;
	out << "}" << endl;
	out << "" << endl;
}

static
void printStack(ostream &out)
{
	out << "/******************************************************************************" << endl;
	out << " * Stack" << endl;
	out << " *****************************************************************************/" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * StackPos" << endl;
	out << " * Index into stack." << endl;
	out << " */" << endl;
	out << "typedef long long StackPos;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * stack" << endl;
	out << " * The stack..." << endl;
	out << " */" << endl;
	out << "State *stack;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * stackPtr" << endl;
	out << " * Index into stack of the first free space." << endl;
	out << " */" << endl;
	out << "StackPos stackPtr;" << endl;
    out << "StackPos maxStackPtr;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * createStack ()" << endl;
	out << " * The function creates a stack." << endl;
	out << " */" << endl;
	out << "void createStack()" << endl;
	out << "{" << endl;
	out << "    stack = (State*)malloc(sizeof(State)*STACK_SIZE);" << endl;
	out << "    " << endl;
	out << "    if (stack == NULL)" << endl;
	out << "        exit(\"Failed creating stack.\", 1);" << endl;
    out << "" << endl;
    out << "    maxStackPtr = 0;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * destroyStack()" << endl;
	out << " * The function destroys a stack." << endl;
	out << " */" << endl;
	out << "void destroyStack()" << endl;
	out << "{" << endl;
	out << "    free(stack);" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * clearStack ()" << endl;
	out << " * Reset the stack to an empty state." << endl;
	out << " */" << endl;
	out << "void clearStack()" << endl;
	out << "{" << endl;
	out << "    stackPtr = 0;   " << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * popStack ()" << endl;
	out << " * Remove last element from the stack." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void popStack()" << endl;
	out << "{" << endl;
	out << "    if (stackPtr > 0)" << endl;
	out << "        stackPtr--;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * pushStack ()" << endl;
	out << " * Put an element on the stack." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void pushStack(const State &s)" << endl;
	out << "{" << endl;
	out << "    if (stackPtr >= STACK_SIZE)" << endl;
	out << "        exit(\"Stack overflow.\", 1);" << endl;
	out << "    " << endl;
	out << "    copyState(stack[stackPtr], s);" << endl;
	out << "    stackPtr++;" << endl;
    out << "" << endl;
    out << "    if (stackPtr > maxStackPtr) maxStackPtr = stackPtr;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * topStack ()" << endl;
	out << " * The function returns the last element of the stack." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "State &topStack()" << endl;
	out << "{" << endl;
	out << "    return stack[stackPtr-1];" << endl;
	out << "}" << endl;
	out << "" << endl;
}

static
void printHash(ostream &out)
{
    out << "/******************************************************************************" << endl;
	out << " * Hash" << endl;
	out << " *****************************************************************************/" << endl;
	out << "" << endl;
	out << "#define INVALID_HASH_KEY    NULL" << endl;
	out << "#define NO_SLOT_IN_HASH     -1" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * HashKey" << endl;
	out << " *" << endl;
	out << " */" << endl;
	out << "typedef unsigned long long HashKey;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * HashSlot" << endl;
	out << " *" << endl;
	out << " */" << endl;
	out << "typedef struct _HashSlot" << endl;
	out << "{" << endl;
	out << "    StackPos value;" << endl;
	out << "    struct _HashSlot *next;" << endl;
	out << "} HashSlot;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * hashTable" << endl;
	out << " * The hash table..." << endl;
	out << " */" << endl;
	out << "HashSlot **hashTable;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * createHashTable ()" << endl;
	out << " * The function constructs a hash table." << endl;
	out << " */" << endl;
	out << "void createHashTable()" << endl;
	out << "{" << endl;
	out << "    hashTable = (HashSlot**)malloc(sizeof(HashSlot*)*HASH_TABLE_SIZE);" << endl;
	out << "    " << endl;
	out << "    if (hashTable == NULL)" << endl;
	out << "        exit(\"Failed creating hash table.\", 1);" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < HASH_TABLE_SIZE; i++)" << endl;
	out << "        hashTable[i] = INVALID_HASH_KEY;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * destroyHashTable ()" << endl;
	out << " * The function destoys a hash table." << endl;
	out << " */" << endl;
	out << "void destroyHashTable()" << endl;
	out << "{" << endl;
	out << "    HashSlot *s_cur, *s_next;" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < HASH_TABLE_SIZE; i++)" << endl;
	out << "    {" << endl;
	out << "        for (s_cur = hashTable[i]; s_cur != INVALID_HASH_KEY; s_cur = s_next)" << endl;
	out << "        {" << endl;
	out << "            s_next = s_cur->next;" << endl;
	out << "            free(s_cur);" << endl;
	out << "        }" << endl;
	out << "    }" << endl;
	out << "" << endl;
	out << "    free(hashTable);" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * clearHashTable ()" << endl;
	out << " * Resets the hash table to contain no keys." << endl;
	out << " */" << endl;
	out << "void clearHashTable()" << endl;
	out << "{" << endl;
	out << "    HashSlot *s_cur, *s_next;" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < HASH_TABLE_SIZE; i++)" << endl;
	out << "    {" << endl;
	out << "        for (s_cur = hashTable[i]; s_cur != INVALID_HASH_KEY; s_cur = s_next)" << endl;
	out << "        {" << endl;
	out << "            s_next = s_cur->next;" << endl;
	out << "            free(s_cur);" << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        hashTable[i] = INVALID_HASH_KEY;" << endl;
	out << "    }" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * hash ()" << endl;
	out << " * The hash function. It is a standard multiplication hash." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "HashKey hash(const State &s)" << endl;
	out << "{" << endl;
	out << "    double key = 0;" << endl;
	out << "    char *s_ptr = (char*)&s;" << endl;
	out << "    long long key_part;" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < sizeof(State); i+=4)" << endl;
	out << "    {" << endl;
	out << "        key_part = 0;" << endl;
	out << "        for (int j = 0; j < 4 && i+j < sizeof(State); j++)" << endl;
	out << "        {" << endl;
	out << "            key_part = key_part << 8; // radix: 128" << endl;
	out << "            key_part = key_part + (long long)(abs(s_ptr[i]));" << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        key = key * 39164205.20662217 + double(key_part) * 0.6180339887;" << endl;
	out << "    }" << endl;
	out << "" << endl;
	out << "    key = fmod(key, 1);" << endl;
	out << "    return (HashKey) floor(HASH_TABLE_SIZE * key);" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * insertKeyHashTable ()" << endl;
	out << " * The function inserts a (key, value) pair into the hash table." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void insertKeyHashTable(const HashKey key, const StackPos value)" << endl;
	out << "{" << endl;
	out << "    HashSlot *slot = (HashSlot*)malloc(sizeof(HashSlot));" << endl;
	out << "    slot->value = value;" << endl;
	out << "    slot->next = hashTable[key];" << endl;
	out << "    " << endl;
	out << "    // Insert the stack position in the hash table" << endl;
	out << "    hashTable[key] = slot;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * findVallueInHashTable ()" << endl;
	out << " * The function returns the StackPos at which the given (key,state) pair is" << endl;
	out << " * located or NO_SLOT_IN_HASH if unkown." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "StackPos findValueInHashTable(const HashKey key, const State &s)" << endl;
	out << "{" << endl;
	out << "    HashSlot *slot;" << endl;
	out << "    " << endl;
	out << "    for (slot = hashTable[key]; slot != INVALID_HASH_KEY; slot = slot->next)" << endl;
	out << "    {" << endl;
	out << "        StackPos pos = slot->value;" << endl;
	out << "        " << endl;
	out << "        if (equalStates(stack[pos], s))" << endl;
	out << "            return pos;" << endl;
	out << "    }" << endl;
	out << "    " << endl;
	out << "    return NO_SLOT_IN_HASH;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * printHashKey ()" << endl;
	out << " * The function outputs the all hash slots with the given key." << endl;
	out << " */" << endl;
	out << "void printHashKey(ostream &out, const HashKey key)" << endl;
	out << "{" << endl;
	out << "    out << \"Key: \" << key <<endl;" << endl;
	out << "    out << endl;" << endl;
	out << "    " << endl;
	out << "    for (HashSlot *slot = hashTable[key]; slot != NULL; slot = slot->next)" << endl;
	out << "    {" << endl;
	out << "        printState(out, stack[slot->value]);" << endl;
	out << "        out << endl;" << endl;
	out << "    }" << endl;
	out << "}" << endl;
    out << endl;
}

static
void printDependencies(ostream &out)
{
	out << "/******************************************************************************" << endl;
	out << " * Dependencies" << endl;
	out << " *****************************************************************************/" << endl;
	out << "" << endl;
	out << "#define NODE_PREV_PERIOD -1" << endl;
	out << "#define NODE_INVALID -2" << endl;
	out << "" << endl;
	out << "typedef long long DepPtr;" << endl;
	out << "" << endl;
	out << "typedef struct _DependencyEdge" << endl;
	out << "{" << endl;
	out << "    bool actor;" << endl;
	out << "    bool ch[SDF_NUM_CHANNELS];" << endl;
	out << "    bool ch_back[SDF_NUM_CHANNELS];" << endl;
	out << "    DepPtr  node;" << endl;
	out << "} DependencyEdge;" << endl;
	out << "" << endl;
	out << "typedef struct _DependencyNode" << endl;
	out << "{" << endl;
	out << "    int actor;" << endl;
	out << "    DepPtr prev;" << endl;
	out << "    bool depFlag;" << endl;
	out << "    DependencyEdge deps[SDF_NUM_ACTORS];" << endl;
	out << "} DependencyNode;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * dependencyStack" << endl;
	out << " * The dependency stack..." << endl;
	out << " */" << endl;
	out << "DependencyNode *dependencyStack;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * dependencyStackPtr" << endl;
	out << " * Index into dependency stack of the first free space." << endl;
	out << " */" << endl;
	out << "DepPtr dependencyStackPtr;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * prevDependencyNode" << endl;
	out << " * The index of the last dependency node created for each actor" << endl;
	out << " * before the current time-step." << endl;
	out << " */" << endl;
	out << "DepPtr prevDependencyNode[SDF_NUM_ACTORS];" << endl;
	out << "DepPtr curDependencyNode[SDF_NUM_ACTORS];" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * dependencyDFScolor" << endl;
	out << " * Array to administer dependency node vists during DFS." << endl;
	out << " * before the current time-step." << endl;
	out << " */" << endl;    
    out << "int *dependencyDFScolor;" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * curReachableNodes" << endl;
    out << " * The array indicates wether an actor b in the current period has a chain" << endl;
    out << " * of dependencies to an actor a in the previous period. If there is" << endl;
    out << " * such a dependency the value of curReachableNodes[b][a] == 1, else it is 0." << endl;
    out << " *" << endl;
    out << " * The 'curReachableNodesCh' array maintains the information which back channels" << endl;
    out << " * are seen on at least one of the paths from b to a." << endl;
    out << " *" << endl;
    out << " * The 'prev' version of both arrays are used during the dependency construction" << endl;
    out << " * process." << endl;
    out << " */" << endl;
    out << "bool curReachableNodes[SDF_NUM_ACTORS][SDF_NUM_ACTORS];" << endl;
    out << "bool prevReachableNodes[SDF_NUM_ACTORS][SDF_NUM_ACTORS];" << endl;
    out << "" << endl;
    out << "bool curReachableNodesCh[SDF_NUM_ACTORS][SDF_NUM_ACTORS][SDF_NUM_CHANNELS];" << endl;
    out << "bool prevReachableNodesCh[SDF_NUM_ACTORS][SDF_NUM_ACTORS][SDF_NUM_CHANNELS];" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * dependenciesDFScolor" << endl;
    out << " * Array to administer dependency node vists during DFS." << endl;
    out << " * before the current time-step." << endl;
    out << " */" << endl;
    out << "int dependenciesDFScolor[SDF_NUM_ACTORS];" << endl;
    out << "" << endl;
	out << "/**" << endl;
	out << " * clearDependencyNode ()" << endl;
	out << " * Set all dependencies in a dependency node to false and the" << endl;
	out << " * previous dependency node pointer is set as invalid." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void clearDependencyNode(DepPtr d)" << endl;
	out << "{" << endl;
	out << "    char *s_ptr = (char*)&dependencyStack[d];" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < sizeof(DependencyNode); i++)" << endl;
	out << "        s_ptr[i] = 0;" << endl;
	out << "    " << endl;
	out << "    dependencyStack[d].prev = NODE_INVALID;" << endl;
	out << "    " << endl;
	out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
	out << "        dependencyStack[d].deps[a].node = NODE_INVALID;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "void printDependencyEdge(ostream &out, DependencyEdge &e)" << endl;
	out << "{" << endl;
	out << "    if (e.node == NODE_INVALID)" << endl;
	out << "        return;" << endl;
	out << "    else if (e.node == NODE_PREV_PERIOD)" << endl;
	out << "        out << \"        dep: <previous period>\" << endl;" << endl;
	out << "    else" << endl;
	out << "        out << \"        dep: \" << e.node << endl;" << endl;
	out << "    " << endl;
	out << "    if (e.actor)" << endl;
	out << "        out << \"        actor:   true\" << endl;" << endl;
	out << "    " << endl;
	out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "        if (e.ch[c])" << endl;
	out << "            out << \"        ch:      \" << c << endl;" << endl;
	out << "" << endl;
	out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "        if (e.ch_back[c])" << endl;
	out << "            out << \"        ch_back: \" << c << endl;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "void printDependencyNode(ostream &out, DependencyNode &d)" << endl;
	out << "{" << endl;
	out << "    out << \"actor:        \" << d.actor << endl;" << endl;
	out << "    " << endl;
	out << "    if (d.prev == NODE_PREV_PERIOD)" << endl;
	out << "        out << \"prev:         <previous period>\" << endl;" << endl;
	out << "    else if (d.prev == NODE_INVALID)" << endl;
	out << "        out << \"prev:         <invalid>\" << endl;" << endl;
	out << "    else" << endl;
	out << "        out << \"prev:         \" << d.prev << endl;" << endl;
	out << "" << endl;
	out << "    out << \"dependencies: \" << endl;" << endl;
	out << "    " << endl;
	out << "    if (d.depFlag == false)" << endl;
	out << "        out << \"    none.\" << endl;" << endl;
	out << "    else" << endl;
	out << "        for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
	out << "        {" << endl;
	out << "            out << \"    to actor: \" << a << endl;" << endl;
	out << "            printDependencyEdge(out, d.deps[a]);" << endl;
	out << "        }" << endl;
	out << "    " << endl;
	out << "    out << endl;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * createDependencyStack ()" << endl;
	out << " * The function creates a dependency stack." << endl;
	out << " */" << endl;
	out << "void createDependencyStack()" << endl;
	out << "{" << endl;
	out << "    dependencyStackPtr = 0;" << endl;
	out << "    dependencyStack = (DependencyNode*) malloc(sizeof(DependencyNode)" << endl;
	out << "                            *DEPENDENCY_STACK_SIZE);" << endl;
	out << "    " << endl;
	out << "    if (dependencyStack == NULL)" << endl;
	out << "        exit(\"Failed creating dependency stack.\", 1);" << endl;
    out << "    " << endl;
	out << "    dependencyDFScolor = (int*) malloc(sizeof(int)" << endl;
	out << "                            *DEPENDENCY_STACK_SIZE);" << endl;
	out << "    " << endl;
	out << "    if (dependencyDFScolor == NULL)" << endl;
	out << "        exit(\"Failed creating dependency stack.\", 2);" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * destroyDependencyStack()" << endl;
	out << " * The function destroys the dependency stack." << endl;
	out << " */" << endl;
	out << "void destroyDependencyStack()" << endl;
	out << "{" << endl;
	out << "    free(dependencyStack);" << endl;
	out << "    free(dependencyDFScolor);" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * clearDependencyStack ()" << endl;
	out << " * Reset the dependency stack to an empty state." << endl;
	out << " */" << endl;
	out << "void clearDependencyStack()" << endl;
	out << "{" << endl;
	out << "    dependencyStackPtr = 0;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * popDependencyStack ()" << endl;
	out << " * Remove last element from the dependency stack." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void popDependencyStack()" << endl;
	out << "{" << endl;
	out << "    if (dependencyStackPtr > 0)" << endl;
	out << "        dependencyStack--;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * pushDependencyStack ()" << endl;
	out << " * Put an element on the dependency stack." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "DepPtr pushDependencyStack()" << endl;
	out << "{" << endl;
	out << "    DepPtr d = dependencyStackPtr;" << endl;
	out << "" << endl;
	out << "    if (dependencyStackPtr >= DEPENDENCY_STACK_SIZE)" << endl;
	out << "        exit(\"Dependency stack overflow.\", 1);" << endl;
	out << "" << endl;
	out << "    clearDependencyNode(dependencyStackPtr);" << endl;
	out << "    dependencyStackPtr++;" << endl;
	out << "        " << endl;
	out << "    return d;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * initDependencies ()" << endl;
	out << " * The function initializes the dependency stack and the last dependency" << endl;
	out << " * node pointers." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void initDependencies()" << endl;
	out << "{" << endl;
	out << "    clearDependencyStack();" << endl;
	out << "    " << endl;
	out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
	out << "    {" << endl;
	out << "        prevDependencyNode[a] = NODE_PREV_PERIOD;" << endl;
	out << "        curDependencyNode[a] = NODE_INVALID;" << endl;
	out << "    }" << endl;
	out << "    " << endl;
	out << "    for (int d = 0; d < DEPENDENCY_STACK_SIZE; d++)" << endl;
	out << "        dependencyDFScolor[d] = 0;" << endl;
    out << "    " << endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "        for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "            if (a == b)" << endl;
    out << "                curReachableNodes[a][b] = true;" << endl;
    out << "            else" << endl;
    out << "                curReachableNodes[a][b] = false;" << endl;
    out << "" << endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "        for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "                curReachableNodesCh[a][b][c] = false;" << endl;
    out << "" << endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "        dependenciesDFScolor[a] = 0;" << endl;
    out << "    " << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * createDependencyNode ()" << endl;
	out << " * Create a new dependency node for the actor a and add the dependency" << endl;
	out << " * to the list 'curDependencyNode'." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "DepPtr createDependencyNode(int a)" << endl;
	out << "{" << endl;
	out << "    DepPtr d;" << endl;
	out << "" << endl;
	out << "    // Get a dependency node from the stack" << endl;
	out << "    d = pushDependencyStack();" << endl;
	out << "    " << endl;
	out << "    dependencyStack[d].actor = a;" << endl;
	out << "    dependencyStack[d].depFlag = false;" << endl;
	out << "    dependencyStack[d].prev = prevDependencyNode[a];" << endl;
	out << "    " << endl;
	out << "    curDependencyNode[a] = d;" << endl;
	out << "    " << endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "        curReachableNodes[b][a] = false;" << endl;
    out << "    " << endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "            curReachableNodesCh[b][a][c] = false;" << endl;
    out << "    " << endl;
	out << "    return d;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * addDependencyEdgeForActor ()" << endl;
	out << " * Add a dependency edge to the dependency node d which expresses a" << endl;
	out << " * dependency of d to the previous firing of actor a." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void addDependencyEdgeForActor(DepPtr d, int a)" << endl;
	out << "{" << endl;
	out << "    // No dependency edge needed if the dependency to which the edge" << endl;
	out << "    // goes has no dependencies and does not belong to the previous period." << endl;
	out << "    DepPtr prev = prevDependencyNode[a];" << endl;
	out << "    if (prev != NODE_PREV_PERIOD && dependencyStack[prev].depFlag == false)" << endl;
	out << "        return;" << endl;
	out << "    " << endl;
	out << "    dependencyStack[d].depFlag = true;" << endl;
	out << "    dependencyStack[d].deps[a].actor = true;" << endl;
	out << "    dependencyStack[d].deps[a].node = prevDependencyNode[a];" << endl;
    out << "    " << endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "        curReachableNodes[b][dependencyStack[d].actor] = " << endl;
    out << "                        curReachableNodes[b][dependencyStack[d].actor]" << endl;
    out << "                            || prevReachableNodes[b][a];" << endl;
    out << "    " << endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "            curReachableNodesCh[b][dependencyStack[d].actor][c] = " << endl;
    out << "                        curReachableNodesCh[b][dependencyStack[d].actor][c] " << endl;
    out << "                            || prevReachableNodesCh[b][a][c];" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * addDependencyEdgeChSpace ()" << endl;
	out << " * Add a dependency edge to the dependency node d which expresses a" << endl;
	out << " * dependency of d to tokens consumed on channel c by the previous firing of" << endl;
	out << " * actor a." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void addDependencyEdgeForChSpace(DepPtr d, int a, int c)" << endl;
	out << "{" << endl;
	out << "    // No dependency edge needed if the dependency to which the edge" << endl;
	out << "    // goes has no dependencies and does not belong to the previous period." << endl;
	out << "    DepPtr prev = prevDependencyNode[a];" << endl;
	out << "    if (prev != NODE_PREV_PERIOD && dependencyStack[prev].depFlag == false)" << endl;
	out << "        return;" << endl;
	out << "    " << endl;
	out << "    dependencyStack[d].depFlag = true;" << endl;
	out << "    dependencyStack[d].deps[a].ch_back[c] = true;" << endl;
	out << "    dependencyStack[d].deps[a].node = prevDependencyNode[a];" << endl;
    out << "    " << endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "        curReachableNodes[b][dependencyStack[d].actor] = " << endl;
    out << "                        curReachableNodes[b][dependencyStack[d].actor]" << endl;
    out << "                            || prevReachableNodes[b][a];" << endl;
    out << "    " << endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "            curReachableNodesCh[b][dependencyStack[d].actor][c] = " << endl;
    out << "                        curReachableNodesCh[b][dependencyStack[d].actor][c] " << endl;
    out << "                            || prevReachableNodesCh[b][a][c];" << endl;
    out << "    " << endl;
    out << "    // Add the back edge to all channels a connects to in previous period" << endl;
    out << "    for (int x = 0; x < SDF_NUM_ACTORS; x++)" << endl;
    out << "    {" << endl;
    out << "        if (curReachableNodes[x][a])" << endl;
    out << "        {" << endl;
    out << "            curReachableNodesCh[x][dependencyStack[d].actor][c] = true;" << endl;
    out << "        } " << endl;
    out << "    }" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * addDependencyEdgeForChTokens ()" << endl;
	out << " * Add a dependency edge to the dependency node d which expresses a" << endl;
	out << " * dependency of d to tokens produced on channel c by the previous firing of" << endl;
	out << " * actor a." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "void addDependencyEdgeForChTokens(DepPtr d, int a, int c)" << endl;
	out << "{" << endl;
	out << "    // No dependency edge needed if the dependency to which the edge" << endl;
	out << "    // goes has no dependencies and does not belong to the previous period." << endl;
	out << "    DepPtr prev = prevDependencyNode[a];" << endl;
	out << "    if (prev != NODE_PREV_PERIOD && dependencyStack[prev].depFlag == false)" << endl;
	out << "        return;" << endl;
	out << "    " << endl;
	out << "    dependencyStack[d].depFlag = true;" << endl;
	out << "    dependencyStack[d].deps[a].ch[c] = true;" << endl;
	out << "    dependencyStack[d].deps[a].node = prevDependencyNode[a];" << endl;
    out << "    " << endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "        curReachableNodes[b][dependencyStack[d].actor] = " << endl;
    out << "                        curReachableNodes[b][dependencyStack[d].actor]" << endl;
    out << "                            || prevReachableNodes[b][a];" << endl;
    out << "    " << endl;
    out << "    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "            curReachableNodesCh[b][dependencyStack[d].actor][c] = " << endl;
    out << "                        curReachableNodesCh[b][dependencyStack[d].actor][c] " << endl;
    out << "                            || prevReachableNodesCh[b][a][c];" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * findAllDependencyPaths ()" << endl;
	out << " * The function performs a DFS to find all paths that go from the actor to" << endl;
	out << " * itself in the previous period (i.e. it looks for a path in the dependency" << endl;
	out << " * nodes from the last dependency node of the actor in the current period to" << endl;
	out << " * a dependency node of the same actor in the previous firing). When such a" << endl;
	out << " * path is found, the list of blocked channels is updated. Requirement is that" << endl;
	out << " * a channel must be blocked on at least one path." << endl;
	out << " */" << endl;
	out << "void findAllDependencyPaths(const int actor, DepPtr d, bool *chP, bool *block)" << endl;
	out << "{" << endl;
	out << "    DependencyNode *n = &dependencyStack[d];" << endl;
	out << "    bool ch[SDF_NUM_CHANNELS];" << endl;
	out << "    " << endl;
	out << "    // Does dependency node have dependencies?" << endl;
	out << "    if (n->depFlag == false)" << endl;
	out << "        return;" << endl;
	out << "    " << endl;
	out << "    // Visited dependency node before?" << endl;
	out << "    if (dependencyDFScolor[d] == 1)" << endl;
	out << "        return;" << endl;
	out << "    " << endl;
	out << "    dependencyDFScolor[d] = 1;" << endl;
	out << "    " << endl;
	out << "    // Recursion" << endl;
	out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
	out << "    {" << endl;
	out << "        // Is there no dependency with a?" << endl;
	out << "        if (n->deps[a].node == NODE_INVALID)" << endl;
	out << "            continue;" << endl;
	out << "    " << endl;
	out << "        // Add all back channels in the dependency to the set ch" << endl;
	out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "            ch[c] = chP[c] || n->deps[a].ch_back[c];" << endl;
	out << "    " << endl;
	out << "        if (n->deps[a].node == NODE_PREV_PERIOD)" << endl;
	out << "        {" << endl;
	out << "            // Found a cycle in the dependencies?" << endl;
	out << "            if (a == actor)" << endl;
	out << "            {" << endl;
	out << "                for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "                    block[c] = block[c] || ch[c];" << endl;
	out << "            }" << endl;
    out << "            else" << endl;
	out << "            {" << endl;
	out << "                // Continue with last dependency node of the actor" << endl;
	out << "                // in the previous period" << endl;
	out << "                findAllDependencyPaths(actor, prevDependencyNode[a], ch, block);" << endl;
	out << "            }" << endl;
	out << "        }" << endl;
	out << "        else" << endl;
	out << "        {" << endl;
	out << "            findAllDependencyPaths(actor, n->deps[a].node, ch, block);" << endl;
	out << "        }" << endl;
	out << "    }" << endl;
    out << "    " << endl;
	out << "    dependencyDFScolor[d] = 0;" << endl;
	out << "}" << endl;
	out << "" << endl;
    out << "/**" << endl;
    out << " * findAllDependencies ()" << endl;
    out << " * The function performs a DFS to find all paths that go from the actor to" << endl;
    out << " * itself in the previous period (i.e. it looks for a path in the dependency" << endl;
    out << " * nodes from the last dependency node of the actor in the current period to" << endl;
    out << " * a dependency node of the same actor in the previous firing). When such a" << endl;
    out << " * path is found, the list of blocked channels is updated. Requirement is that" << endl;
    out << " * a channel must be blocked on at least one path." << endl;
    out << " */" << endl;
    out << "void findAllDependencies(const int actor, const int b, bool *chP, bool *block)" << endl;
    out << "{" << endl;
    out << "    bool ch[SDF_NUM_CHANNELS];" << endl;
    out << "    " << endl;
    out << "    // Visited node b before?" << endl;
    out << "    if (dependenciesDFScolor[b])" << endl;
    out << "    {" << endl;
    out << "        // Is the node b the actor we are looking for?" << endl;
    out << "        if (actor == b)" << endl;
    out << "        {" << endl;
    out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "                block[c] = block[c] || chP[c];" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        return;" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    dependenciesDFScolor[b] = 1;" << endl;
    out << "    " << endl;
    out << "    // Iterate over all nodes to find nodes reachable from b" << endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "    {" << endl;
    out << "        // Is node a reachable from b?" << endl;
    out << "        if (curReachableNodes[b][a])" << endl;
    out << "        {" << endl;
    out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "                ch[c] = chP[c] || curReachableNodesCh[b][a][c];" << endl;
    out << "                " << endl;
    out << "            findAllDependencies(actor, a, ch, block);" << endl;
    out << "        }" << endl;
    out << "    } " << endl;
    out << "    " << endl;
    out << "    dependenciesDFScolor[b] = 0;" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printSdfHeader(TimedSDFgraph *g, ostream &out, CString &outputActor)
{
    out << "/******************************************************************************" << endl;
	out << " * SDF" << endl;
	out << " *****************************************************************************/" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * outputIntervalCycle ()" << endl;
	out << " * The function calculates the output interval of the states on the cycle. Its" << endl;
	out << " * value is equal to the average time between two firings in the cycle." << endl;
	out << " */" << endl;
	out << "inline" << endl;
	out << "TTime outputIntervalCycle(const StackPos cyclePos)" << endl;
	out << "{" << endl;
	out << "    int nr_fire = 0;" << endl;
	out << "    TTime time = 0;" << endl;
	out << "" << endl;
	out << "    // Check all state from stack till cycle complete" << endl;
	out << "    for (StackPos ptr = stackPtr -1; ptr >= cyclePos; ptr--)" << endl;
	out << "    {" << endl;
	out << "        // Output actor fired?" << endl;
	
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        
        if (outputActor == a->getName())
        {
            out << "        if (stack[ptr].act_clk[";
            out << a->getId(); 
            out << "] == 1)" << endl;
       }
    }
 
    out << "            nr_fire++;" << endl;
	out << "" << endl;
	out << "        // Time between previous state" << endl;
	out << "        time += stack[ptr].glb_clk;" << endl;
	out << "    }" << endl;
	out << "" << endl;
	out << "    return time/nr_fire;" << endl;
	out << "}" << endl;
	out << "" << endl;
    out << "/**" << endl;
    out << " * storeState ()" << endl;
    out << " * The function stores the given state at the stack and insert an entry" << endl;
    out << " * into the hash table. Cycle detection is performed, as well as a" << endl;
    out << " * check to verify wether a cycle is valid or not." << endl;
    out << " *" << endl;
    out << " * Return values:" << endl;
    out << " *      -1      - added state to stack" << endl;
    out << " *      >= 0    - output interval of detected cycle" << endl;
    out << " */" << endl;
    out << "inline" << endl;
    out << "TTime storeState(State &sdfState)" << endl;
    out << "{" << endl;
    out << "    // Calculate the key in the hash table" << endl;
    out << "    HashKey key = hash(sdfState);" << endl;
    out << "" << endl;
    out << "    // Check occurance in hash table" << endl;
    out << "    StackPos stackPos = findValueInHashTable(key, sdfState);" << endl;
    out << "    if (stackPos == NO_SLOT_IN_HASH)" << endl;
    out << "    {" << endl;
    out << "        // State not in hash table (i.e. state not seen before)" << endl;
    out << "        // Insert state in hash table" << endl;
    out << "        insertKeyHashTable(key, stackPtr);" << endl;
    out << "        " << endl;
    out << "        // Insert state in stack" << endl;
    out << "        pushStack(sdfState);" << endl;
    out << "    }" << endl;
    out << "    else" << endl;
    out << "    {" << endl;
    out << "        // State has been visited before (i.e. found cycle)" << endl;
    out << "        return outputIntervalCycle(stackPos);" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    return -1;" << endl;
    out << "}" << endl;
	out << "" << endl;
	out << "#define GLB_CLK             sdfState.glb_clk" << endl;
	out << "#define ACT_CLK(a)          sdfState.act_clk[a]" << endl;
	out << "#define CH(c)               sdfState.ch[c]" << endl;
	out << "" << endl;
	out << "#define ACT_READY(a)        (ACT_CLK(a) == 0)" << endl;
	out << "#define CH_TOKENS(c,n)      (CH(c) >= n)" << endl;
	out << "#define CH_SPACE(c,n)       (CH(c) <= sz[c] - n)" << endl;
	out << "#define FIRE_ACT(a,t)       ACT_CLK(a) = t;" << endl;
	out << "#define CONSUME(c,n)        CH(c) = CH(c) - n;" << endl;
	out << "#define PRODUCE(c,n)        CH(c) = CH(c) + n;" << endl;
	out << "#define LOWER_CLK(a)        if (ACT_CLK(a) > 0) { ACT_CLK(a) = ACT_CLK(a) - 1; }" << endl;
	out << "#define ACT_END_FIRE(a)     (ACT_CLK(a) == 1)" << endl;
	out << "#define ADVANCE_CLK         GLB_CLK = GLB_CLK + 1;" << endl;
	out << "#define NEXT_ITER           GLB_CLK = 0;" << endl;
	out << "" << endl;
	out << "#define ACT_READY_PREV(a)   (prevStateP.act_clk[a] == 0)" << endl;
	out << "#define CH_TOKENS_PREV(c,n) (prevStateP.ch[c] >= n)" << endl;
	out << "#define CH_SPACE_PREV(c,n)  (prevStateP.ch[c] <= sz[c] - n)" << endl;
	out << "" << endl;
	out << "State sdfState;" << endl;
	out << "State prevState;" << endl;
	out << "State prevStateP;" << endl;
	out << "" << endl;
}

static
void printAnalyzePeriodicPhase(TimedSDFgraph *g, ostream &out, CString &outputActor)
{
    out << "/**" << endl;
    out << " * analyzePeriodicPhase ()" << endl;
    out << " * Analyze the periodic phase of the schedule to find all critical cycles." << endl;
    out << " */" << endl;
    out << "void analyzePeriodicPhase(const TBufSize *sz, bool *block)" << endl;
    out << "{" << endl;
    out << "    State periodicState;" << endl;
    out << "    DepPtr d;" << endl;
    out << "    " << endl;
    out << "    // Current sdf state is a periodic state" << endl;
    out << "    copyState(periodicState, sdfState);" << endl;
    out << "    " << endl;
    out << "    // Initialize the dependencies" << endl;
    out << "    initDependencies();" << endl;
    out << "    " << endl;
    out << "    // Start new iteration of the periodic phase" << endl;
    out << "    NEXT_ITER;" << endl;
    out << "    " << endl;
    out << "    // Complete the remaining actor firings" << endl;

    bool foundOutputActor = false;
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        SDFactor *a = *iter;
            
        if (foundOutputActor)
        {
            out << "        if (ACT_END_FIRE(" << a->getId() << "))" << endl;
            out << "        {" << endl;

            for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                    pIter++)
            {
                SDFport *p = *pIter;
                SDFchannel *ch = p->getChannel();

                if (p->getType() == SDFport::In)
                {
                    out << "            CONSUME(" << ch->getId() << ",";
                    out << p->getRate() << ");" << endl;
                }    
                else
                {
                    out << "            PRODUCE(" << ch->getId() << ",";
                    out << p->getRate() << ");" << endl;
                }
            }
            out << "        }" << endl;
        }

        // Actor a is the output actor?
        if (outputActor == a->getName())
            foundOutputActor = true;

    }
    out << "    " << endl;

    out << "    // Fire the actors" << endl;
    out << "    while (true)" << endl;
    out << "    {" << endl;

    out << "        // Lower clocks" << endl;
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        SDFactor *a = *iter;
        out << "        LOWER_CLK(" << a->getId() << ");" << endl;
    }
    out << "" << endl;

    out << "        // Advance the global clock" << endl;
    out << "        ADVANCE_CLK;" << endl;
    out << "        " << endl;
    out << "        // Update previous actor dependencies" << endl;
    out << "        for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "            for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "                prevReachableNodes[a][b] = curReachableNodes[a][b];" << endl;
    out << "        " << endl;
    out << "        for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "            for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "                for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "                prevReachableNodesCh[a][b][c] = curReachableNodesCh[a][b][c];" << endl;
    out << "        " << endl;
    out << "        // Start actor firings" << endl;
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;

        out << "        if (ACT_READY(" << a->getId() << "))" << endl;
        out << "        {" << endl;
        out << "            bool fire = true;" << endl;
        out << "            " << endl;
        out << "            // Check space on each output" << endl;
        
        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::Out)
            {
                out << "            if (!CH_SPACE(" << ch->getId() << "," << p->getRate();
                out << "))" << endl;
                out << "                fire = false;" << endl;               
            }
        }

        out << "            " << endl;
        out << "            // Check tokens on each input" << endl;

        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::In)
            {
                out << "            if (!CH_TOKENS(" << ch->getId() << "," << p->getRate();
                out << "))" << endl;
                out << "                fire = false;" << endl;
            }
        }

        out << "            " << endl;
        out << "            // Ready to fire?" << endl;
        out << "            if (fire)" << endl;
        out << "            {" << endl;
        out << "                // Create a dependency node for this actor firing" << endl;
        out << "                d = createDependencyNode(" << a->getId() << ");" << endl;
        out << "                " << endl;
        out << "                // Which dependencies were resolved in previous time step?" << endl;



        out << "                if (!ACT_READY_PREV(" << a->getId() << "))" << endl;
        out << "                {" << endl;
        out << "                    // Previous firing of actor was not finished" << endl;
        out << "                    addDependencyEdgeForActor(d," << a->getId() << ");" << endl;
        out << "                }" << endl;
        out << "                else" << endl;
        out << "                {" << endl;
        out << "                    // Check tokens and space on each channel" << endl;

        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::Out)
            {
                out << "                    if (!CH_SPACE_PREV(" << ch->getId() << "," << p->getRate();
                out << "))" << endl;
                out << "                        addDependencyEdgeForChSpace(d,";
                out << ch->oppositePort(p)->getActor()->getId() << ",";
                out << ch->getId() << ");" << endl;               
            }
            else
            {
                out << "                    if (!CH_TOKENS_PREV(" << ch->getId() << "," << p->getRate();
                out << "))" << endl;
                out << "                        addDependencyEdgeForChTokens(d,";
                out << ch->oppositePort(p)->getActor()->getId() << ",";
                out << ch->getId() << ");" << endl;               
            }
        }

        out << "                }" << endl;
        out << "                " << endl;
        out << "                // Fire actor" << endl;
        out << "                FIRE_ACT(" << a->getId() << ",";
        out << a->getExecutionTime() << ");" << endl;
        out << "            }" << endl;
        out << "        }" << endl;
    }
    out << "        " << endl;

    out << "        // Update previous actor dependency nodes" << endl;
    out << "        for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "        {" << endl;
    out << "            // Current node is a valid node?" << endl;
    out << "            if (curDependencyNode[a] != NODE_INVALID)" << endl;
    out << "            {" << endl;
    out << "                prevDependencyNode[a] = curDependencyNode[a];" << endl;
    out << "                curDependencyNode[a] = NODE_INVALID;" << endl;
    out << "            }" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        // Store state to check for progress" << endl;
    out << "        copyState(prevStateP, sdfState);" << endl;
    out << "        " << endl;

    out << "        // Finish actor firings" << endl;
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        SDFactor *a = *iter;

        out << "        if (ACT_END_FIRE(" << a->getId() << "))" << endl;
        out << "        {" << endl;
        
        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::In)
            {
                out << "            CONSUME(" << ch->getId() << ",";
                out << p->getRate() << ");" << endl;
            }    
            else
            {
                out << "            PRODUCE(" << ch->getId() << ",";
                out << p->getRate() << ");" << endl;
            }
        }

        if (outputActor == a->getName())
        {
            out << "" << endl;
            out << "            // Reached periodic state?" << endl;
            out << "            if (equalStates(periodicState, sdfState))" << endl;
            out << "                break;" << endl;
            out << "            " << endl;
	        out << "            NEXT_ITER;" << endl;
        }
    
        out << "        }" << endl;
    }

    out << "    }" << endl;
    out << "    " << endl;
    out << "    // Print all dependencies" << endl;
    out << "    //for (DepPtr d = 0; d < dependencyStackPtr; d++)" << endl;
    out << "    //{" << endl;
    out << "    //    cerr << \"Dependency: \" << d << endl;" << endl;
    out << "    //    printDependencyNode(cerr, dependencyStack[d]);" << endl;
    out << "    //}" << endl;
    out << "    //cerr << endl;" << endl;
    out << "    " << endl;
    out << "    // Print reachable nodes" << endl;
    out << "    //for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "    //{" << endl;
    out << "    //    for (int b = 0; b < SDF_NUM_ACTORS; b++)" << endl;
    out << "    //    {" << endl;
    out << "    //        if (curReachableNodes[a][b])" << endl;
    out << "    //        {" << endl;
    out << "    //            cerr << \"prev(\" << a << \") -> act(\" << b << \")\" << endl;" << endl;
    out << "    //            " << endl;
    out << "    //            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "    //            {" << endl;
    out << "    //                if (curReachableNodesCh[a][b][c])" << endl;
    out << "    //                    cerr << \"   ch: \" << c << endl;" << endl;
    out << "    //            }" << endl;
    out << "    //            cerr << endl;" << endl;
    out << "    //        }" << endl;
    out << "    //    }" << endl;
    out << "    //}" << endl;
    out << "    " << endl;
    out << "    // Find all paths from an actor to itself in the state-space" << endl;
    out << "    // This is a path from the actor to a dependency of the same actor" << endl;
    out << "    // in a previous period (This version does not find the critical cycle)." << endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "    {" << endl;
    out << "        bool ch[SDF_NUM_CHANNELS];" << endl;
    out << "        " << endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "            ch[c] = false;" << endl;
    out << "        " << endl;
    out << "        findAllDependencies(a, a, ch ,block);" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    // Find all paths from an actor to itself in the state-space" << endl;
    out << "    // This is a path from the last dependency node (i.e. entry in the" << endl;
    out << "    // prevDependencyNode array) to a dependency of the same node in a" << endl;
    out << "    // previous period (This version finds the critical cycle)." << endl;
    out << "    //for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "    //{" << endl;
    out << "    //    bool ch[SDF_NUM_CHANNELS];" << endl;
    out << "    //    " << endl;
    out << "    //    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "    //        ch[c] = false;" << endl;
    out << "    //" << endl;
    out << "    //    findAllDependencyPaths(a, prevDependencyNode[a], ch ,block);" << endl;
    out << "    //}" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static 
void printAnalyzeDeadlock(TimedSDFgraph *g, ostream &out, CString &outputActor)
{
    out << "/**" << endl;
    out << " * analyzeDeadlock ()" << endl;
    out << " * Analyze the deadlock in the schedule." << endl;
    out << " */" << endl;
    out << "void analyzeDeadlock(const TBufSize *sz, bool *block)" << endl;
    out << "{" << endl;
    out << "    DepPtr d;" << endl;
    out << "    " << endl;
    out << "    // Initialize the dependencies" << endl;
    out << "    initDependencies();" << endl;
    out << "    " << endl;
    out << "    // Create a dependency node for each actor to find" << endl;
    out << "    // the cause of it blocking." << endl;
    
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;

        out << "    // Actor: " << a->getId() << endl;

        out << "    // Create a dependency node for this actor firing" << endl;
        out << "    d = createDependencyNode(" << a->getId() << ");" << endl;
        out << "    dependencyStack[d].depFlag = true;" << endl;
        out << "    " << endl;
        out << "    // Check space on each output" << endl;        

        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::Out)
            {
                out << "    if (!CH_SPACE(" << ch->getId() << "," << p->getRate();
                out << "))" << endl;
                out << "        addDependencyEdgeForChSpace(d,";
                out << ch->oppositePort(p)->getActor()->getId() << ",";
                out << ch->getId() << ");" << endl;               
            }
        }

        out << "    " << endl;
        out << "    // Check tokens on each input" << endl;

        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::In)
            {
                out << "    if (!CH_TOKENS(" << ch->getId() << "," << p->getRate();
                out << "))" << endl;
                out << "        addDependencyEdgeForChTokens(d,";
                out << ch->oppositePort(p)->getActor()->getId() << ",";
                out << ch->getId() << ");" << endl;               
            }
        }
    }
    
    out << "    // Update previous actor dependency nodes" << endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "    {" << endl;
    out << "        // Current node is a valid node?" << endl;
    out << "        if (curDependencyNode[a] != NODE_INVALID)" << endl;
    out << "        {" << endl;
    out << "            prevDependencyNode[a] = curDependencyNode[a];" << endl;
    out << "            curDependencyNode[a] = NODE_INVALID;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    // Print all dependencies" << endl;
    out << "    //for (DepPtr d = 0; d < dependencyStackPtr; d++)" << endl;
    out << "    //{" << endl;
    out << "    //    cerr << \"Dependency: \" << d << endl;" << endl;
    out << "    //    printDependencyNode(cerr, dependencyStack[d]);" << endl;
    out << "    //}" << endl;
    out << "    //cerr << endl;" << endl;
    out << "    " << endl;
    out << "    // Find all paths from an actor to itself in the state-space" << endl;
    out << "    // This is a path from the last dependency node (i.e. entry in the" << endl;
    out << "    // prevDependencyNode array) to a dependency of the same node in the" << endl;
    out << "    // previous period." << endl;
    out << "    for (int a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "    {" << endl;
    out << "        bool ch[SDF_NUM_CHANNELS];" << endl;
    out << "        " << endl;
    out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
    out << "            ch[c] = false;" << endl;
    out << "    " << endl;
    out << "        findAllDependencyPaths(a, prevDependencyNode[a], ch ,block);" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printExecSdfGraph(TimedSDFgraph *g, ostream &out, CString &outputActor)
{
    out << "/**" << endl;
    out << " * execSDFGraph()" << endl;
    out << " * Execute the SDF graph till a deadlock is found or a recurrent state. The output interval" << endl;
    out << " * (i.e. inverse of throughput) is returned." << endl;
    out << " */" << endl;
    out << "TTime execSDFGraph(const TBufSize *sz, bool *block)" << endl;
    out << "{" << endl;
    out << "    TTime output_interval;" << endl;
    out << "    " << endl;
    out << "    // Create initial state and add it to stack" << endl;
    out << "    clearState(sdfState);" << endl;
    out << "    copyState(prevState, sdfState);" << endl;
    out << "    prevState.glb_clk = TTIME_MAX;" << endl;
    out << "    " << endl;
    out << "    // Initial tokens" << endl;

    for (SDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        SDFchannel *ch = *iter;

        if (ch->getInitialTokens() != 0)
        {
            out << "    if (!CH_SPACE(" << ch->getId() << ",";
            out << ch->getInitialTokens() <<"))" << endl;
            out << "    {" << endl;
            out << "        block[" << ch->getId() << "] = true;" << endl;
            out << "        return TTIME_MAX;" << endl;
            out << "    }" << endl;
            out << "    PRODUCE(" << ch->getId() << ",";
            out << ch->getInitialTokens() <<");" << endl;
        }
    }

    out << "" << endl;
    out << "    // Fire the actors" << endl;
    out << "    while (true)" << endl;
    out << "    {" << endl;
    out << "        // Finish actor firings" << endl;

    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        SDFactor *a = *iter;

        out << "        if (ACT_END_FIRE(" << a->getId() << "))" << endl;
        out << "        {" << endl;
        
        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::In)
            {
                out << "            CONSUME(" << ch->getId() << ",";
                out << p->getRate() << ");" << endl;
            }    
            else
            {
                out << "            PRODUCE(" << ch->getId() << ",";
                out << p->getRate() << ");" << endl;
            }
        }

        if (outputActor == a->getName())
        {
            out << "" << endl;
            out << "            // Add state to hash of visited states" << endl;
            out << "            output_interval = storeState(sdfState);" << endl;
            out << "            if (output_interval != -1)" << endl;
            out << "            {" << endl;
            out << "#ifndef SEARCH_FULL_SPACE" << endl;
            out << "                analyzePeriodicPhase(sz, block);" << endl;
            out << "#endif" << endl;
            out << "                return output_interval;" << endl;
            out << "            }" << endl;
	        out << "            NEXT_ITER;" << endl;
        }
    
        out << "        }" << endl;
    }
    
    out << endl;

    out << "        // Lower clocks" << endl;
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        SDFactor *a = *iter;
        out << "        LOWER_CLK(" << a->getId() << ");" << endl;
    }
    out << "" << endl;

	out << "        // Check for progress (i.e. no deadlock)" << endl;
	out << "        if (equalStates(prevState, sdfState))" << endl;
 	out << "        {" << endl;
 	out << "            // Find cause of deadlock" << endl;
 	out << "            analyzeDeadlock(sz, block);" << endl;
 	out << "            " << endl;
 	out << "            return TTIME_MAX;" << endl;
	out << "        }" << endl;
    out << "        " << endl;

    out << "        // Advance the global clock" << endl;
    out << "        ADVANCE_CLK;" << endl;
    out << "        " << endl;

	out << "        // Store state to check for progress" << endl;
	out << "        copyState(prevState, sdfState);" << endl;
    out << "        " << endl;

    out << "        // Start actor firings" << endl;
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;

        out << "        if (ACT_READY(" << a->getId() << "))" << endl;
        out << "        {" << endl;
        out << "            bool fire = true;" << endl;
        out << "            " << endl;

        out << "            // Check tokens on each input" << endl;
        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::In)
            {
                out << "            if (!CH_TOKENS(" << ch->getId() << "," << p->getRate() << "))" << endl;
                out << "                fire = false;" << endl;             
            }
        }
        out << "            " << endl;

        out << "            // Check space on each output" << endl;
        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::Out)
            {
                out << "            if (!CH_SPACE(" << ch->getId() << "," << p->getRate() << "))" << endl;
                out << "                fire = false;" << endl;             
            }
        }
        out << "            " << endl;

        out << "            // Ready to fire?" << endl;
        out << "            if (fire)" << endl;
        out << "                FIRE_ACT(" << a->getId() << ",";
        out << a->getExecutionTime() << ");" << endl;
        out << "        }" << endl;
    }
    out << "        " << endl;
    out << "        // Store state to find actor activity in periodic phase" << endl;
    out << "        copyState(prevStateP, sdfState);" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printSdf(TimedSDFgraph *g, ostream &out, CString &outputActor)
{
    printSdfHeader(g, out, outputActor);
    printAnalyzePeriodicPhase(g, out, outputActor);
    printAnalyzeDeadlock(g, out, outputActor);
    printExecSdfGraph(g, out, outputActor);
}

static
void printDistribution(ostream &out)
{
    out << "/*******************************************************************************" << endl;
    out << " * Distributions" << endl;
    out << " ******************************************************************************/" << endl;
	out << "typedef struct _Distribution" << endl;
	out << "{" << endl;
	out << "    TBufSize size;" << endl;
	out << "    TTime outputInterval;" << endl;
	out << "    TBufSize sz[SDF_NUM_CHANNELS];" << endl;
	out << "    bool block[SDF_NUM_CHANNELS];" << endl;
	out << "    struct _Distribution *next;" << endl;
	out << "    struct _Distribution *check;" << endl;
	out << "} Distribution;" << endl;
	out << "" << endl;
	out << "typedef struct _MinStorageDistr" << endl;
	out << "{" << endl;
	out << "    TBufSize size;" << endl;
	out << "    TTime   outputInterval;" << endl;
	out << "    Distribution *distributions;" << endl;
	out << "    struct _MinStorageDistr *next;" << endl;
	out << "    struct _MinStorageDistr *prev;" << endl;
	out << "} MinStorageDistr;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * minStorageDistributions" << endl;
	out << " * List of all minimal storage distributions." << endl;
	out << " */" << endl;
	out << "MinStorageDistr *minStorageDistributions = NULL;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * checkedDistributions" << endl;
	out << " * Lists of all checked distributions." << endl;
	out << " */" << endl;
	out << "Distribution *checkedDistributions = NULL;" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * execDistribution ()" << endl;
	out << " * Execute one distribution." << endl;
	out << " */" << endl;
	out << "void execDistribution(Distribution *d)" << endl;
	out << "{" << endl;
	out << "    // Clear stack and hash table" << endl;
	out << "    clearStack();" << endl;
	out << "    clearHashTable();" << endl;
	out << "    " << endl;
	out << "    // Initialize blocking channels" << endl;
	out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "        d->block[c] = false;" << endl;
	out << "    " << endl;
	out << "    // Execute the SDF graph to find its output interval" << endl;
	out << "    d->outputInterval = execSDFGraph(d->sz, d->block);" << endl;
	out << "    " << endl;    
	out << "    //cerr << d->size << \" \" << d->outputInterval << endl;" << endl;
	out << "    //for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "    //    cerr << d->sz[c] << \" \" << d->block[c] << endl;" << endl;
	out << "    //cerr << endl;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * findLbOutputInterval ()" << endl;
	out << " * Find the lower bound on the output interval by executing the" << endl;
	out << " * graph with the upper bounds on the channel sizes." << endl;
	out << " */" << endl;
	out << "TTime findLbOutputInterval()" << endl;
	out << "{" << endl;
	out << "    TTime outputInterval;" << endl;
	out << "    Distribution d;" << endl;
	out << "    " << endl;
	out << "    // Create distribution with upper bounds" << endl;
	out << "    d.next = NULL;" << endl;
	out << "    d.check = NULL;" << endl;
	out << "    d.size = UB_DISTRIBUTION_SZ;" << endl;
	out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "        d.sz[c] = maxSz[c];" << endl;
	out << "" << endl;
	out << "    // Execute distribution to find output interval" << endl;
	out << "    execDistribution(&d);" << endl;
	out << "    outputInterval = d.outputInterval;" << endl;
	out << "    " << endl;
	out << "    return outputInterval;" << endl;
	out << "}" << endl;
 	out << "" << endl;
	out << "/**" << endl;
	out << " * graphDeadlockFree()" << endl;
	out << " * The function checks wether the SDF graph is deadlock free. If not," << endl;
	out << " * false is returned. Else, the function returns true." << endl;
	out << " */" << endl;
	out << "bool graphDeadlockFree()" << endl;
	out << "{" << endl;
	out << "    TTime outputInterval;" << endl;
	out << "    " << endl;
	out << "    // Find output interval for unbounded graph" << endl;
	out << "    outputInterval = findLbOutputInterval();" << endl;
	out << "    "<< endl;
	out << "    if (outputInterval == TTIME_MAX)" << endl;
	out << "        return false;" << endl;
	out << "    "<< endl;
	out << "    return true;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * updateMinimalDistributions ()" << endl;
	out << " * Add the distribution d to the set of minimal storage" << endl;
	out << " * distribution (if it is one) and remove all distributions" << endl;
	out << " * which can not be a minimal storage distribution because of d." << endl;
	out << " */" << endl;
	out << "void updateMinimalDistributions(Distribution *d)" << endl;
	out << "{" << endl;
	out << "    MinStorageDistr *dist = minStorageDistributions;" << endl;
	out << "    MinStorageDistr *distn;" << endl;
	out << "    " << endl;
	out << "    // Infinite output interval (deadlock)?" << endl;
	out << "    if (d->outputInterval == TTIME_MAX)" << endl;
	out << "        return;" << endl;
	out << "    " << endl;
	out << "    while (dist != NULL)" << endl;
	out << "    {" << endl;
	out << "        // Larger throughput with larger size" << endl;
	out << "        if (d->outputInterval < dist->outputInterval" << endl;
	out << "                && d->size > dist->size)" << endl;
	out << "        {" << endl;
	out << "            if (dist->next == NULL) break;" << endl;
	out << "            " << endl;
	out << "            // Next distribution" << endl;
	out << "            dist = dist->next;" << endl;
	out << "            continue;" << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        // Smaller throughput with equal or larger size or" << endl;
	out << "        // equal throughput with larger size" << endl;
	out << "        if ((d->outputInterval > dist->outputInterval && d->size >= dist->size)" << endl;
	out << "                || (d->outputInterval == dist->outputInterval " << endl;
	out << "                        && d->size > dist->size))" << endl;
	out << "        {" << endl;
	out << "            // Distribution d is not minimal" << endl;
	out << "            return;        " << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        // Equal throughput and size" << endl;
	out << "        if (d->outputInterval == dist->outputInterval && d->size == dist->size)" << endl;
	out << "        {" << endl;
	out << "            // Add distribution d to dist" << endl;
	out << "            d->next = dist->distributions;" << endl;
	out << "            dist->distributions = d;" << endl;
	out << "            " << endl;
	out << "            // done" << endl;
	out << "            return;" << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        // Smaller throughput with a smaller size" << endl;
	out << "        if (d->outputInterval > dist->outputInterval && d->size < dist->size)" << endl;
	out << "        {" << endl;
	out << "            // Add distribution d before dist" << endl;
	out << "            distn = new MinStorageDistr;" << endl;
	out << "            distn->outputInterval = d->outputInterval;" << endl;
	out << "            distn->size = d->size;" << endl;
	out << "            distn->distributions = d;" << endl;
	out << "            distn->prev = dist->prev;" << endl;
	out << "            distn->next = dist;" << endl;
	out << "            if (dist->prev != NULL)" << endl;
	out << "                dist->prev->next = distn;" << endl;
	out << "            dist->prev = distn;" << endl;
	out << "        " << endl;
	out << "            if (distn->prev == NULL)" << endl;
	out << "                minStorageDistributions = distn;" << endl;
	out << "            " << endl;
	out << "            // done" << endl;
	out << "            return;            " << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        // Equal throughput with smaller size or " << endl;
	out << "        // larger throughput with smaller or equal size" << endl;
	out << "        if ((d->outputInterval == dist->outputInterval && d->size < dist->size)" << endl;
	out << "                || (d->outputInterval < dist->outputInterval " << endl;
	out << "                        && d->size <= dist->size))" << endl;
	out << "        {" << endl;
	out << "            // Distribution d is minimal, but dist is not minimal " << endl;
	out << "            // (and possibly other after d are also not minimal)" << endl;
	out << "            distn = new MinStorageDistr;" << endl;
	out << "            distn->outputInterval = d->outputInterval;" << endl;
	out << "            distn->size = d->size;" << endl;
	out << "            distn->distributions = d;" << endl;
	out << "            distn->prev = dist->prev;" << endl;
	out << "            if (dist->prev != NULL)" << endl;
	out << "                dist->prev->next = distn;" << endl;
	out << "           " << endl;
	out << "            if (distn->prev == NULL)" << endl;
	out << "                minStorageDistributions = distn;" << endl;
	out << "" << endl;
	out << "            while (dist != NULL && (d->outputInterval < dist->outputInterval " << endl;
	out << "                    || (d->outputInterval == dist->outputInterval " << endl;
	out << "                        && d->size < dist->size)))" << endl;
	out << "                dist = dist->next;" << endl;
	out << "            " << endl;
	out << "            distn->next = dist;" << endl;
	out << "            if (dist != NULL)" << endl;
	out << "                dist->prev = distn;" << endl;
	out << "            " << endl;
	out << "            // done" << endl;
	out << "            return;" << endl;
	out << "        }" << endl;
	out << "    }" << endl;
	out << "    " << endl;
	out << "    // Add storage distribution to the end of the list" << endl;
	out << "    distn = new MinStorageDistr;" << endl;
	out << "    distn->outputInterval = d->outputInterval;" << endl;
	out << "    distn->size = d->size;" << endl;
	out << "    distn->distributions = d;" << endl;
	out << "    distn->next = NULL;" << endl;
	out << "    " << endl;
	out << "    if (minStorageDistributions == NULL)" << endl;
	out << "    {" << endl;
	out << "        distn->prev = NULL;" << endl;
	out << "        minStorageDistributions = distn;" << endl;
	out << "    }" << endl;
	out << "    else" << endl;
	out << "    {" << endl;
	out << "        distn->prev = dist;" << endl;
	out << "        dist->next = distn;" << endl;
	out << "    }" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * checkedDistribution ()" << endl;
	out << " * The function returns true if the distribution d has been" << endl;
	out << " * executed before, else it returns false." << endl;
	out << " */" << endl;
	out << "bool checkedDistribution(Distribution *d)" << endl;
	out << "{" << endl;
	out << "    Distribution *dist = checkedDistributions;" << endl;
	out << "    " << endl;
	out << "    while (dist != NULL)" << endl;
	out << "    {" << endl;
	out << "        bool equal = true;" << endl;
	out << "        " << endl;
	out << "        // Compare distributions d and dist" << endl;
	out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "        {" << endl;
	out << "            if (d->sz[c] != dist->sz[c])" << endl;
	out << "            {" << endl;
	out << "                equal = false;" << endl;
	out << "                break;" << endl;
	out << "            }" << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        // Distributions equal?" << endl;
	out << "        if (equal)" << endl;
	out << "            return true;" << endl;
	out << "           " << endl;
	out << "        // Next distribution" << endl;
	out << "        dist = dist->check;" << endl;
	out << "    }" << endl;
	out << "    " << endl;
	out << "    return false;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * checkDistribution ()" << endl;
	out << " * Compute the output interval of a distribution and enlarge" << endl;
	out << " * all channels which block execution (recursively check those" << endl;
	out << " * distributions). After computing the output interval, the" << endl;
	out << " * list of minimal storage distributions is updated." << endl;
	out << " */" << endl;
	out << "void checkDistribution(Distribution *d)" << endl;
	out << "{" << endl;
	out << "    Distribution *dn;" << endl;
	out << "    " << endl;
	out << "    // Add d to list of checked distributions" << endl;
	out << "    d->check = checkedDistributions;" << endl;
	out << "    checkedDistributions = d;" << endl;
	out << "    " << endl;
	out << "    // Compute output interval of this distribution" << endl;
	out << "    execDistribution(d);" << endl;
    out << "    " << endl;
    out << "    if (d->outputInterval == lbOutputInterval" << endl;
    out << "            && d->size < maxDistributionSz)" << endl;
    out << "        maxDistributionSz = d->size;" << endl;
    out << "    " << endl;
	out << "    // Add the distribution to the set of minimal distributions" << endl;
	out << "    updateMinimalDistributions(d);    " << endl;
	out << "" << endl;
	out << "    // Enlarge each of the blocked channels" << endl;
	out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "    {" << endl;
	out << "        if (d->block[c])" << endl;
	out << "        {" << endl;
	out << "            if (d->sz[c] < maxSz[c])" << endl;
	out << "            {" << endl;
	out << "                // Create a new distribution" << endl;
	out << "                dn = new Distribution;" << endl;
	out << "                dn->next = NULL;" << endl;
	out << "                dn->size = d->size + minSzStep[c];" << endl;
	out << "                for (int ch = 0; ch < SDF_NUM_CHANNELS; ch++)" << endl;
	out << "                    dn->sz[ch] = d->sz[ch];" << endl;
	out << "                dn->sz[c] += minSzStep[c];" << endl;
	out << "                " << endl;
	out << "                // Distribution checked before or too large?" << endl;
	out << "                if (dn->size > maxDistributionSz || checkedDistribution(dn))" << endl;
	out << "                {" << endl;
	out << "                    delete dn;" << endl;
	out << "                }" << endl;
	out << "                else" << endl;
	out << "                {" << endl;
	out << "                    // Check distribution" << endl;
	out << "                    checkDistribution(dn);" << endl;
	out << "                }" << endl;
	out << "            }" << endl;
	out << "        }" << endl;
	out << "    }" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * findMinimalStorageDistributions ()" << endl;
	out << " * The function computes all minimal storage distributions." << endl;
	out << " */" << endl;
	out << "void findMinimalStorageDistributions()" << endl;
	out << "{" << endl;
	out << "    Distribution *d;" << endl;
	out << "    " << endl;
    out << "    // Check deadlock in graph" << endl;
    out << "    if (!graphDeadlockFree())" << endl;
    out << "        exit(\"Graph is not deadlock free.\",1);" << endl;
    out << "    " << endl;
    out << "    // Find lower bound of the output interval" << endl;
    out << "    lbOutputInterval = findLbOutputInterval();" << endl;
    out << "" << endl;
	out << "    // Create distribution with lower bound" << endl;
	out << "    d = new Distribution;" << endl;
	out << "    d->next = NULL;" << endl;
	out << "    d->size = LB_DISTRIBUTION_SZ;" << endl;
	out << "    for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "        d->sz[c] = minSz[c];" << endl;
	out << "    " << endl;
	out << "    // Check the distribution" << endl;
	out << "    checkDistribution(d);" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * checkAllDistributions ()" << endl;
	out << " * Check all distributions within the given margins." << endl;
	out << " */" << endl;
	out << "void checkAllDistributions(short c, TBufSize usedSz, TBufSize requiredMinSz," << endl;
	out << "        TBufSize distributionSz, MinStorageDistr *distr)" << endl;
	out << "{" << endl;
	out << "    static Distribution d;" << endl;
	out << "    " << endl;
	out << "    // Last channel - end of recursion" << endl;
	out << "    if (c == (SDF_NUM_CHANNELS - 1))" << endl;
	out << "    {" << endl;
	out << "        d.sz[c] = distributionSz - usedSz;" << endl;
	out << "" << endl;
	out << "        // Compute output interval of this distribution" << endl;
	out << "        execDistribution(&d);" << endl;
	out << "        " << endl;
	out << "        // Smaller then current minimum?" << endl;
	out << "        if (d.outputInterval < distr->outputInterval)" << endl;
	out << "        {" << endl;
	out << "            // Remove existing distributions" << endl;
	out << "            while (distr->distributions != NULL)" << endl;
	out << "            {" << endl;
	out << "                Distribution *dn = distr->distributions;" << endl;
	out << "                distr->distributions = dn->next;" << endl;
	out << "                delete dn;" << endl;
	out << "            }" << endl;
	out << "            " << endl;
	out << "            // Create first distribution with this output interval" << endl;
	out << "            Distribution *dn = new Distribution;" << endl;
	out << "            dn->next = NULL;" << endl;
	out << "            for (int ch = 0; ch < SDF_NUM_CHANNELS; ch++)" << endl;
	out << "                dn->sz[ch] = d.sz[ch];" << endl;
	out << "            distr->distributions = dn;" << endl;
	out << "            distr->outputInterval = d.outputInterval;" << endl;
	out << "        }" << endl;
	out << "        else if (d.outputInterval == distr->outputInterval)" << endl;
	out << "        {" << endl;
	out << "            // Add alternative distribution" << endl;
	out << "            Distribution *dn = new Distribution;" << endl;
	out << "            dn->next = NULL;" << endl;
	out << "            for (int ch = 0; ch < SDF_NUM_CHANNELS; ch++)" << endl;
	out << "                dn->sz[ch] = d.sz[ch];" << endl;
	out << "            dn->next = distr->distributions;" << endl;
	out << "            distr->distributions = dn;" << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        // done" << endl;
	out << "        return;" << endl;
	out << "        " << endl;
	out << "    }" << endl;
	out << "        " << endl;
	out << "    // Enlarge channel c" << endl;
	out << "    for (d.sz[c] = minSz[c]; (d.sz[c] + usedSz) <=" << endl;
	out << "            (distributionSz-(requiredMinSz-minSz[c])) && d.sz[c] <= maxSz[c]; " << endl;
	out << "                d.sz[c] = d.sz[c] + minSzStep[c])" << endl;
	out << "    {" << endl;
	out << "        // usedSz must be smaller as distributionSz" << endl;
	out << "        if (usedSz+d.sz[c] <= distributionSz)" << endl;
	out << "            checkAllDistributions(c+1, usedSz+d.sz[c], requiredMinSz-minSz[c]," << endl;
	out << "                        distributionSz, distr);" << endl;
	out << "    }" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * findMimimumOutputInterval ()" << endl;
	out << " * The function checks all storage distributions with |distribution| =" << endl;
	out << " * maxDistributionSz to find the minimal output interval." << endl;
	out << " */" << endl;
	out << "MinStorageDistr *findMinimumOutputInterval(TBufSize distributionSz)" << endl;
	out << "{" << endl;
	out << "    MinStorageDistr *distr = new MinStorageDistr;" << endl;
	out << "    TBufSize requiredMinSz = 0;" << endl;
	out << "    " << endl;
	out << "    // Initialize minimal storage distribution" << endl;
	out << "    distr->outputInterval = TTIME_MAX;" << endl;
	out << "    distr->size = distributionSz;" << endl;
	out << "    distr->distributions = NULL;" << endl;
	out << "    distr->next = NULL;" << endl;
	out << "    distr->prev = NULL;" << endl;
	out << "    " << endl;
	out << "    // Set the minimal size of all fifos" << endl;
	out << "#ifdef SEARCH_FULL_SPACE" << endl;
	out << "    if (false)" << endl;
	out << "#else" << endl;
	out << "    if (minStorageDistributions != NULL)" << endl;
	out << "#endif" << endl;
	out << "    {" << endl;
	out << "        for (Distribution *d = minStorageDistributions->distributions;" << endl;
	out << "                d != NULL; d = d->next)" << endl;
	out << "        {" << endl;
	out << "            // Set minimal size of all channels and " << endl;
	out << "            // which channels must be enlarged" << endl;
	out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "                minSz[c] = d->sz[c];" << endl;
	out << "            " << endl;
	out << "            // Lower bound distribution size" << endl;
	out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "                requiredMinSz += minSz[c];" << endl;
	out << "            " << endl;
	out << "            checkAllDistributions(0, 0, requiredMinSz, distributionSz, distr);" << endl;
	out << "        }" << endl;
	out << "    }" << endl;
	out << "    else" << endl;
	out << "    {" << endl;
	out << "        // Lower bound distribution size" << endl;
	out << "        for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "            requiredMinSz += minSz[c];" << endl;
	out << "        " << endl;
	out << "        checkAllDistributions(0, 0, requiredMinSz, distributionSz, distr);" << endl;
	out << "    }" << endl;
	out << "    " << endl;
	out << "    return distr;" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * findMinimalStorageDistributions ()" << endl;
	out << " * The function uses linear search from the lower bound to the upper bound" << endl;
	out << " * to find all minimal storage distributions." << endl;
	out << " */" << endl;
	out << "void findMinimalStorageDistributions(TBufSize lb_distr_sz, TBufSize ub_distr_sz)" << endl;
	out << "{" << endl;
	out << "    TBufSize m;" << endl;
	out << "    MinStorageDistr *d;" << endl;
	out << "" << endl;
	out << "    // Start with lower bound" << endl;
	out << "    m = lb_distr_sz;" << endl;
	out << "    " << endl;
    out << "    // Find lower bound of the output interval" << endl;
    out << "    lbOutputInterval = findLbOutputInterval();" << endl;
    out << "    " << endl;
	out << "    do" << endl;
	out << "    {" << endl;
	out << "        // Find smallest output interval with distribution size m." << endl;
	out << "        d = findMinimumOutputInterval(m);" << endl;
	out << "" << endl;
	out << "        // Is set distributions d a set of minimal distributions?" << endl;
	out << "        if (minStorageDistributions == NULL || " << endl;
	out << "                d->outputInterval < minStorageDistributions->outputInterval)" << endl;
	out << "        {" << endl;
	out << "            // Distribution d has output interval smaller than infinite?" << endl;
	out << "            if (d->outputInterval < TTIME_MAX)" << endl;
	out << "            {" << endl;
	out << "                // Add d to the list of minimal distributions" << endl;
	out << "                d->next = minStorageDistributions;" << endl;
	out << "                minStorageDistributions = d;" << endl;
	out << "            }" << endl;
    out << "            else" << endl;
    out << "            {" << endl;
	out << "                // Distribution d is not minimal, remove it" << endl;
	out << "                while(d->distributions != NULL)" << endl;
	out << "                {" << endl;
	out << "                    Distribution *t = d->distributions;" << endl;
	out << "                    d->distributions = t->next;" << endl;
	out << "                    delete t;" << endl;
	out << "                }" << endl;
	out << "                delete d;" << endl;
    out << "            }" << endl;
	out << "        }" << endl;
	out << "        else" << endl;
	out << "        {" << endl;
	out << "            // Distribution d is not minimal, remove it" << endl;
	out << "            while(d->distributions != NULL)" << endl;
	out << "            {" << endl;
	out << "                Distribution *t = d->distributions;" << endl;
	out << "                d->distributions = t->next;" << endl;
	out << "                delete t;" << endl;
	out << "            }" << endl;
	out << "            delete d;" << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        // Lower bound output interval reached?        " << endl;
	out << "        if (minStorageDistributions != NULL &&" << endl;
	out << "            minStorageDistributions->outputInterval <= lbOutputInterval)" << endl;
	out << "            return;" << endl;
	out << "   " << endl;
	out << "        // Next distribution size" << endl;
	out << "        m++;" << endl;
	out << "    } while (ub_distr_sz >= m);" << endl;
	out << "}" << endl;
	out << "" << endl;
	out << "/**" << endl;
	out << " * printMinimalDistributions ()" << endl;
	out << " * Output all minimal distributions to the supplied stream." << endl;
	out << " */" << endl;
	out << "void printMinimalDistributions(std::ostream &out)" << endl;
	out << "{" << endl;
	out << "    out << \"<?xml version='1.0' encoding='UTF-8'?>\" << endl;" << endl;
	out << "    out << \"<!DOCTYPE buffy >\" << endl;" << endl;
	out << "    out << \"<buffy\";" << endl;
    out << "    out << \" nrChannels='\" << SDF_NUM_CHANNELS << \"'\";" << endl;
	out << "    out << \" lbDistributionSz='\" << LB_DISTRIBUTION_SZ;" << endl;
	out << "    out << \"' ubDistributionSz='\" << UB_DISTRIBUTION_SZ;" << endl;
	out << "    out << \"' maxStates='\" << maxStackPtr << \"'>\" << endl;" << endl;
	out << "" << endl;
	out << "    for (MinStorageDistr *p = minStorageDistributions; p != NULL;" << endl;
	out << "            p = p->next)" << endl;
	out << "    {" << endl;
	out << "        out << \"    <minDistributions outputInterval='\" << p->outputInterval;" << endl;
	out << "        out << \"' distributionSz='\" << p->size << \"'>\" << endl;" << endl;
	out << "        for (Distribution *d = p->distributions; d != NULL; d = d->next)" << endl;
	out << "        {" << endl;
	out << "            out << \"        <distribution>\" << endl;" << endl;
	out << "            for (int c = 0; c < SDF_NUM_CHANNELS; c++)" << endl;
	out << "            {" << endl;
	out << "                out << \"            <ch id='\" << c << \"' sz='\" << d->sz[c];" << endl;
	out << "                out << \"'/>\" << endl;" << endl;
	out << "            }" << endl;
	out << "            out << \"        </distribution>\" << endl;" << endl;
	out << "        }" << endl;
	out << "        out << \"    </minDistributions>\" << endl;" << endl;
	out << "    }" << endl;
	out << "    out << \"</buffy>\" << endl;" << endl;
	out << "}" << endl;
	out << "" << endl;
}

static
void printMain(ostream &out, bool paretoSpace)
{
	out << "int main(int argc, char **argv)" << endl;
	out << "{" << endl;
    out << "    // Check bounds" << endl;
    out << "    if (LB_DISTRIBUTION_SZ > UB_DISTRIBUTION_SZ)" << endl;
    out << "        exit(\"lb_distributin_sz > ub_distribution_sz\", 1);" << endl;
    out << "" << endl;
	out << "    // Create hash and stack" << endl;
	out << "    createStack();" << endl;
	out << "    createHashTable();" << endl;
    out << "    createDependencyStack();" << endl;
	out << "" << endl;

    if (paretoSpace)
    {
	    out << "    // Search the space    " << endl;
	    out << "#ifdef SEARCH_FULL_SPACE" << endl;
	    out << "    findMinimalStorageDistributions(LB_DISTRIBUTION_SZ, UB_DISTRIBUTION_SZ);" << endl;
	    out << "#else" << endl;
	    out << "    findMinimalStorageDistributions();" << endl;
	    out << "#endif" << endl;
	    out << "    " << endl;
	    out << "    // Output all minimal storage distributions" << endl;
        out << "    printMinimalDistributions(cout);" << endl;
    }
    else
    {
        out << "    // Find the maximal throughput" << endl;
        out << "    TTime lbOutputInterval = findLbOutputInterval();" << endl;
	    out << "    " << endl;
	    out << "    cout << \"lb output interval: \" << lbOutputInterval << endl;" << endl;
    }
            
    out << "    " << endl;
	out << "    // Cleanup" << endl;
	out << "    destroyStack();" << endl;
	out << "    destroyHashTable();" << endl;
    out << "    destroyDependencyStack();" << endl;
	out << " " << endl;
	out << "    return 0;" << endl;
	out << "}" << endl;
}

extern
void outputSDFasBuffyModel(TimedSDFgraph *g, ostream &out,
        CString &outputActor, unsigned long long stackSz,
        unsigned long long hashSz, unsigned long long depStackSz)
{
    printDefinitions(g, out, stackSz, hashSz, depStackSz);
    printMiscFunctions(out);
    printState(out);
    printStack(out);
    printHash(out);
    printDependencies(out);
    printSdf(g, out, outputActor);
    printDistribution(out);
    printMain(out, true);
}

extern
void outputSDFasBuffyModelThroughput(TimedSDFgraph *g, ostream &out,
        CString &outputActor, unsigned long long stackSz, 
        unsigned long long hashSz, unsigned long long depStackSz)
{
    printDefinitions(g, out, stackSz, hashSz, depStackSz);
    printMiscFunctions(out);
    printState(out);
    printStack(out);
    printHash(out);
    printDependencies(out);
    printSdf(g, out, outputActor);
    printDistribution(out);
    printMain(out, false);
}
