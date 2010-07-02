/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffy_autoconc.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 17, 2005
 *
 *  Function        :   Buffy model for throughput calculcation with
 *                      auto-concurrency.
 *
 *  History         :
 *      17-11-05    :   Initial version.
 *
 * $Id: buffy_autoconc.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

static
void printDefinitions(TimedSDFgraph *g, ostream &out, 
        unsigned long long stackSz, unsigned long long hashSz,
        unsigned long long repCntOutputActor)
{
    out << "#define SDF_NUM_ACTORS          " << g->nrActors() << endl;
    out << "#define SDF_NUM_CHANNELS        " << g->nrChannels() << endl;
    out << "#define SDF_MAX_TIME            " << getMaxExecTime(g) << endl;
    out << "" << endl;
    out << "#define SDF_OUT_ACTOR_REP_CNT   " << repCntOutputActor << endl;
    out << "" << endl;
    out << "#define STACK_SIZE              " << stackSz << endl;
    out << "#define HASH_TABLE_SIZE         " << hashSz << endl;
    out << "" << endl;
    out << "typedef long TBufSize;" << endl;
    out << "typedef short TCnt;" << endl;
    out << "typedef double TTime;" << endl;
    out << "" << endl;
    out << "#define TTIME_MAX INT_MAX" << endl;
    out << "" << endl;
}

static
void printMiscFunctions(ostream &out)
{
    out << "#include <math.h>" << endl;
    out << "#include <iostream>" << endl;
    out << "#include <assert.h>" << endl;
    out << "#include <time.h>" << endl;
    out << "#include <sys/resource.h>" << endl;
    out << "" << endl;
    out << "using std::ostream;" << endl;
    out << "using std::cout;" << endl;
    out << "using std::cerr;" << endl;
    out << "using std::endl;" << endl;
    out << "" << endl;
    out << "typedef struct _CTimer" << endl;
    out << "{" << endl;
    out << "    struct rusage rStart;" << endl;
    out << "    struct rusage rEnd;" << endl;
    out << "    struct timeval time;" << endl;
    out << "} CTimer;" << endl;
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
    out << "/**" << endl;
    out << " * startTimer ()" << endl;
    out << " * The function start measuring the elapsed user and system time " << endl;
    out << " * from this point onwards." << endl;
    out << " */" << endl;
    out << "static inline" << endl;
    out << "void startTimer(CTimer *t)" << endl;
    out << "{" << endl;
    out << "    getrusage(RUSAGE_SELF, &(t->rStart));" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * stopTimer ()" << endl;
    out << " * The function stops measuring the elapsed user and system time and" << endl;
    out << " * it computes the total elapsed time. This time is accesible via " << endl;
    out << " * t->time (struct timeval)." << endl;
    out << " */" << endl;
    out << "static inline" << endl;
    out << "void stopTimer(CTimer *t)" << endl;
    out << "{" << endl;
    out << "    getrusage(RUSAGE_SELF, &(t->rEnd));" << endl;
    out << "" << endl;
    out << "    // Calculate elapsed time (user + system)" << endl;
    out << "    t->time.tv_sec = t->rEnd.ru_utime.tv_sec - t->rStart.ru_utime.tv_sec" << endl;
    out << "                        + t->rEnd.ru_stime.tv_sec - t->rEnd.ru_stime.tv_sec;" << endl;
    out << "    t->time.tv_usec = t->rEnd.ru_utime.tv_usec - t->rStart.ru_utime.tv_usec " << endl;
    out << "                        + t->rEnd.ru_stime.tv_usec - t->rStart.ru_stime.tv_usec;" << endl;
    out << "" << endl;
    out << "    if (t->time.tv_sec >= 1e6)" << endl;
    out << "    {" << endl;
    out << "        t->time.tv_sec = t->time.tv_sec + t->time.tv_usec/long(1e6);" << endl;
    out << "        t->time.tv_usec = t->time.tv_usec%long(1e6);" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * printTimer ()" << endl;
    out << " * The function prints the value of the timer in milliseconds to the supplied" << endl;
    out << " * output stream." << endl;
    out << " */" << endl;
    out << "static inline" << endl;
    out << "void printTimer(ostream &out, CTimer *t)" << endl;
    out << "{" << endl;
    out << "    double s = (t->time.tv_sec * (double)(1000)) " << endl;
    out << "                    + (t->time.tv_usec / (double)(1000));" << endl;
    out << "    " << endl;
    out << "    out << s << \"ms\";" << endl;
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
	out << "    TCnt act_clk[SDF_NUM_ACTORS][SDF_MAX_TIME+1];" << endl;
	out << "    TTime glb_clk;" << endl;
	out << "    TBufSize ch[SDF_NUM_CHANNELS];" << endl;
	out << "    TBufSize ckt[SDF_NUM_CHANNELS];" << endl;
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
	out << "    {" << endl;
	out << "        out << \"act_clk[\" << i << \"] = {\";" << endl;
	out << "        " << endl;
	out << "        for (int j = 0; j < SDF_MAX_TIME+1; j++)" << endl;
	out << "        {" << endl;
	out << "            if (s.act_clk[i][j] != 0)" << endl;
	out << "                out << \"(\" << s.act_clk[i][j] << \",\" << j << \"), \";" << endl;
	out << "        }" << endl;
	out << "        " << endl;
	out << "        out << \"}\" << endl;" << endl;
	out << "    }" << endl;
	out << "    " << endl;
	out << "    out << \"glb_clk = \" << s.glb_clk << endl;" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < SDF_NUM_CHANNELS; i++)" << endl;
	out << "        out << \"ch[\" << i << \"] = \" << s.ch[i] << endl;" << endl;
	out << "    " << endl;
	out << "    for (int i = 0; i < SDF_NUM_CHANNELS; i++)" << endl;
	out << "        out << \"ckt[\" << i << \"] = \" << s.ckt[i] << endl;" << endl;
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
            out << "        if (stack[ptr].act_clk[" << a->getId() << "][0] > 0)" << endl;
            out << "            nr_fire++;" << endl;
       }
    }
 
	out << "" << endl;
	out << "        // Time between previous state" << endl;
	out << "        time += stack[ptr].glb_clk;" << endl;
	out << "    }" << endl;
    out << "    " << endl;
	out << "    return time/(TTime)(nr_fire);" << endl;
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
	out << "#define ACT_CLK(a,t)        sdfState.act_clk[a][t]" << endl;
	out << "#define CH(c)               sdfState.ch[c]" << endl;
	out << "#define CKT(c)              sdfState.ckt[c]" << endl;
	out << "" << endl;
	out << "#define FIRE_ACT(a,t)       ACT_CLK(a,t)++;" << endl;
	out << "#define FIRE_ACT_END(a)     ACT_CLK(a,0)--;" << endl;
	out << "#define CH_TOKENS(c,n)      (CH(c)-CKT(c) >= n)" << endl;
	out << "#define CK_TOKENS(c,n)      CKT(c) = CKT(c) + n;" << endl;
	out << "#define CONSUME(c,n)        CH(c) = CH(c) - n; CKT(c) = CKT(c) - n;" << endl;
	out << "#define PRODUCE(c,n)        CH(c) = CH(c) + n;" << endl;
	out << "#define ACT_END_FIRE(a)     (ACT_CLK(a,0) != 0)" << endl;
	out << "#define ADVANCE_CLK         GLB_CLK = GLB_CLK + 1;" << endl;
	out << "#define NEXT_ITER           GLB_CLK = 0;" << endl;
	out << "#define LOWER_CLK(a)        memmove(&ACT_CLK(a,0),&ACT_CLK(a,1), \\" << endl;
	out << "                                sizeof(TCnt)*SDF_MAX_TIME); \\" << endl;
	out << "                            ACT_CLK(a,SDF_MAX_TIME) = 0;" << endl;
	out << "" << endl;
	out << "State sdfState;" << endl;
	out << "State prevState;" << endl;
	out << "" << endl;
}

static
void printExecSdfGraph(TimedSDFgraph *g, ostream &out, CString &outputActor)
{
    out << "/**" << endl;
    out << " * execSDFGraph()" << endl;
    out << " * Execute the SDF graph till a deadlock is found or a recurrent state." << endl;
    out << " * The output interval (i.e. inverse of throughput) is returned." << endl;
    out << " */" << endl;
    out << "TTime execSDFGraph()" << endl;
    out << "{" << endl;
    out << "    TTime output_interval;" << endl;
    out << "    int repCnt = 0;" << endl;
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

        out << "        while (ACT_END_FIRE(" << a->getId() << "))" << endl;
        out << "        {" << endl;

        if (outputActor == a->getName())
        {
            out << "            repCnt++;" << endl;
            out << "            if (repCnt == SDF_OUT_ACTOR_REP_CNT)" << endl;
            out << "            {" << endl;
            out << "                // Add state to hash of visited states" << endl;
            out << "                output_interval = storeState(sdfState);" << endl;
            out << "                if (output_interval != -1)" << endl;
            out << "                    return output_interval;" << endl;
	        out << "                NEXT_ITER;" << endl;
            out << "                repCnt = 0;" << endl;
            out << "            }" << endl;
            out << "            " << endl;
        }
        
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

        out << "            FIRE_ACT_END(" << a->getId() << ");" << endl;
        out << "        }" << endl;
    }
    out << "        " << endl;
    
    out << "        // Start actor firings" << endl;
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;

        out << "        while (true";
        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::In)
            {
                out << " && CH_TOKENS(" << ch->getId() << "," << p->getRate() << ")";
            }
        }
        out << ")" << endl;

        out << "        {" << endl;
        out << "            // Claim tokens on each input" << endl;
        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::In)
            {
                out << "            CK_TOKENS(" << ch->getId() << "," << p->getRate() << ");" << endl;
            }
        }
        out << "            " << endl;
        out << "            // Fire the actor" << endl;
        out << "            FIRE_ACT(" << a->getId() << ",";
        out << a->getExecutionTime() << ");" << endl;
        out << "        }" << endl;
    }
    out << "        " << endl;

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
 	out << "            return TTIME_MAX;" << endl;
    out << "        " << endl;

    out << "        // Advance the global clock" << endl;
    out << "        ADVANCE_CLK;" << endl;
    out << "        " << endl;

	out << "        // Store state to check for progress" << endl;
	out << "        copyState(prevState, sdfState);" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printSdf(TimedSDFgraph *g, ostream &out, CString &outputActor)
{
    printSdfHeader(g, out, outputActor);
    printExecSdfGraph(g, out, outputActor);
}

static
void printMain(ostream &out)
{
	out << "int main(int argc, char **argv)" << endl;
	out << "{" << endl;
	out << "    // Create hash and stack" << endl;
	out << "    createStack();" << endl;
	out << "    createHashTable();" << endl;
	out << "    " << endl;
    out << "    // Find the maximal throughput" << endl;
    out << "    cout << \"thr = \" << execSDFGraph() << endl;" << endl;
    out << "    " << endl;
	out << "    // Cleanup" << endl;
	out << "    destroyStack();" << endl;
	out << "    destroyHashTable();" << endl;
	out << "    " << endl;
	out << "    return 0;" << endl;
	out << "}" << endl;
}

/**
 * outputSDFasStateSpaceThroughputModel ()
 * Output the SDF graph as a buffy model to find the maximal
 * throughput for unconstrained buffer sizes and using auto-concurrency.
 */
extern
void outputSDFasStateSpaceThroughputModel(TimedSDFgraph *g, ostream &out,
        unsigned long long stackSz, unsigned long long hashSz)
{
    SDFactor *outputActor;
    RepetitionVector repVec;
    CString outActor;
    
    // Find the output actor and its repetition count
    repVec = computeRepetitionVector(g);
    outputActor = findOutputActor(g, repVec);
    outActor = outputActor->getName();
    
    printDefinitions(g, out, stackSz, hashSz, repVec[outputActor->getId()]);
    printMiscFunctions(out);
    printState(out);
    printStack(out);
    printHash(out);
    printSdf(g, out, outActor);
    printMain(out);
}
