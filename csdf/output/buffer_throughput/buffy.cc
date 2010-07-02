/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   buffy.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   June 25, 2007
 *
 *  Function        :   Buffer sizing for CSDFGs
 *
 *  History         :
 *      25-06-07    :   Initial version.
 *
 * $Id: buffy.cc,v 1.3 2008/03/22 14:24:21 sander Exp $
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

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

static
void printMinimalChannelSzStep(TimedCSDFgraph *g, ostream &out)
{
    bool first = true;

    for (CSDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        uint minStepSz;
        CSDFchannel *ch = *iter;
        CSDFport *srcPort = ch->getSrcPort();
        CSDFport *dstPort = ch->getDstPort();
        
        // Initialize minStepSz with a "randomly" selected rate from all
        // possible rates
        minStepSz = srcPort->getRate()[0];
        
        // Step size is equal to the gcd of all producation and consumption
        // rates that are possible
        for (uint i = 0; i < srcPort->getRate().size(); i++)
            minStepSz = gcd(minStepSz, srcPort->getRate()[i]);
        for (uint i = 0; i < dstPort->getRate().size(); i++)
            minStepSz = gcd(minStepSz, dstPort->getRate()[i]);

        if (first)
        {
            out << "{";
            first = false;
        }
        else
        {
            out << ", ";
        }
        out << "Q * " << minStepSz;
    }
    out << "}";
}

static
void printMinimalChannelSz(TimedCSDFgraph *g, ostream &out)
{
    bool first = true;
    uint minSzCh;

    for (CSDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        CSDFchannel *ch = *iter;
        CSDFport *srcPort = ch->getSrcPort();
        CSDFport *dstPort = ch->getDstPort();
        uint ratePeriod = gcd(srcPort->getRate().size(),
                                dstPort->getRate().size());
        uint minStepSz;
        
        // Initialize minStepSz with a "randomly" selected rate from all
        // possible rates
        minStepSz = srcPort->getRate()[0];
        
        // Step size is equal to the gcd of all producation and consumption
        // rates that are possible
        for (uint i = 0; i < srcPort->getRate().size(); i++)
            minStepSz = gcd(minStepSz, srcPort->getRate()[i]);
        for (uint i = 0; i < dstPort->getRate().size(); i++)
            minStepSz = gcd(minStepSz, dstPort->getRate()[i]);

        // Initialize lower bound to maximal size
        minSzCh = UINT_MAX;
        
        // Iterate over all production/consumption rate combinations to find
        // lower bound as given for SDFG case.
        for (uint i = 0; i < ratePeriod; i++)
        {
            uint p = srcPort->getRate()[i % srcPort->getRate().size()];
            uint c = dstPort->getRate()[i % dstPort->getRate().size()];
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
                if (gcd(p,c) != 0)
                    lb = p+c-gcd(p,c)+t%gcd(p,c);
                else
                    lb = p+c-gcd(p,c);
                lb = (lb > t ? lb : t);
            }

            if (lb < minSzCh)
                minSzCh = lb;
        }

        // Minimal channel size must be a multiple of the minStepSz
        minSzCh = minSzCh - (minSzCh % minStepSz);

        if (first)
        {
            first = false;
            out << "{" << minSzCh;
        }
        else
        {
            out << ", " << minSzCh;
        }
    }
    out << "}";
}

static
uint getLbDistributionSz(TimedCSDFgraph *g)
{
    uint minSz = 0;
    uint minSzCh;

    for (CSDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        CSDFchannel *ch = *iter;
        CSDFport *srcPort = ch->getSrcPort();
        CSDFport *dstPort = ch->getDstPort();
        uint ratePeriod = gcd(srcPort->getRate().size(),
                                dstPort->getRate().size());
        uint minStepSz;
        
        // Initialize minStepSz with a "randomly" selected rate from all
        // possible rates
        minStepSz = srcPort->getRate()[0];
        
        // Step size is equal to the gcd of all producation and consumption
        // rates that are possible
        for (uint i = 0; i < srcPort->getRate().size(); i++)
            minStepSz = gcd(minStepSz, srcPort->getRate()[i]);
        for (uint i = 0; i < dstPort->getRate().size(); i++)
            minStepSz = gcd(minStepSz, dstPort->getRate()[i]);

        // Initialize lower bound to maximal size
        minSzCh = UINT_MAX;
        
        // Iterate over all production/consumption rate combinations to find
        // lower bound as given for SDFG case.
        for (uint i = 0; i < ratePeriod; i++)
        {
            uint p = srcPort->getRate()[i % srcPort->getRate().size()];
            uint c = dstPort->getRate()[i % dstPort->getRate().size()];
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
                if (gcd(p,c) != 0)
                    lb = p+c-gcd(p,c)+t%gcd(p,c);
                else
                    lb = p+c-gcd(p,c);
                lb = (lb > t ? lb : t);
            }

            if (lb < minSzCh)
                minSzCh = lb;
        }

        // Minimal channel size must be a multiple of the minStepSz
        minSzCh = minSzCh - (minSzCh % minStepSz);
        
        minSz += minSzCh;
    }

    return minSz;
}

static
CId getOutputActor(TimedCSDFgraph *g)
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
    
    return a->getId();
}

static
uint getOutputActorRepCnt(TimedCSDFgraph *g)
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
    
    return repVec[a->getId()];
}

static
double getMaxThroughput(TimedCSDFgraph *g)
{
    CSDFstateSpaceThroughputAnalysis thrAlgo;
    
    return thrAlgo.analyze(g);
}

static
uint computeGraphPeriod(TimedCSDFgraph *g)
{
    uint period = 1;
    
    for (CSDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        CSDFchannel *c = *iter;
        
        period = lcm(period, (uint)c->getSrcPort()->getRate().size());
        period = lcm(period, (uint)c->getDstPort()->getRate().size());
    }

    for (CSDFactorsIter iter = g->actorsBegin();
            iter != g->actorsEnd(); iter++)
    {
        TimedCSDFactor *a = (TimedCSDFactor*)(*iter);
     
        period = lcm(period, (uint)a->getExecutionTime().size());   
    }
    
    return period;
}

static
uint computeNrActorsHSDFG(TimedCSDFgraph *g)
{
    CSDFgraph::RepetitionVector repVec = g->getRepetitionVector();
    uint cnt = 0;
    
    for (uint i = 0; i < repVec.size(); i++)
    {
        cnt += repVec[i];
    }
    
    return cnt;
}

static
void printDefinitions(TimedCSDFgraph *g, ostream &out)
{
    bool first;
    uint period = computeGraphPeriod(g);

    out << "#include <float.h>" << endl;
    out << "#include <math.h>" << endl;
    out << "#include <assert.h>" << endl;
    out << "#include <iostream>" << endl;
    out << "#include <list>" << endl;
    out << "#include <time.h>" << endl;
    out << "#include <sys/resource.h>" << endl;
    out << "" << endl;
    out << "using namespace std;" << endl;
    out << "" << endl;
    
    out << "typedef unsigned int   TCnt;" << endl;
    out << "typedef unsigned int   TTime;" << endl;
    out << "typedef unsigned int   TBufSize;" << endl;
    out << "typedef unsigned short CId;" << endl;
    out << "typedef double TDtime;" << endl;
    out << endl;

    out << "// Quantization factor of the step size" << endl;
    out << "#ifndef Q" << endl;
    out << "#define Q 1" << endl;
    out << "#endif" << endl;
    out << endl;

    out << "#define TDTIME_MAX              DBL_MAX" << endl;
    out << "#define TBUFSIZE_MAX            ULONG_MAX" << endl;
    out << endl;

    out << "#define max(a,b)                ((a)>(b) ? (a) : (b))" << endl;
    out << "#define min(a,b)                ((a)<(b) ? (a) : (b))" << endl;
    out << endl;

    out << "#define SRC                     0" << endl;
    out << "#define DST                     1" << endl;
    out << "" << endl;

    out << "#define NR_ACTORS               " << g->nrActors() << endl;
    out << "#define NR_CHANNELS             " << g->nrChannels() << endl;
    out << "#define PERIOD                  " << period << endl;
    out << endl;

    out << "#define NR_ACTORS_HSDFG         " << computeNrActorsHSDFG(g) << endl;
    out << endl;
    
    out << "uint channel[" << g->nrChannels() << "][2] = {";
    
    first = true;
    for (CSDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        CSDFchannel *c = *iter;
        
        if (!first)
            out << ", ";
        first = false;
        
        out << "{" << c->getSrcActor()->getId() << ", ";
        out << c->getDstActor()->getId() << "}";
    }

    out << "};" << endl;            
    
    out << "uint channelInitialTokens[" << g->nrChannels() << "] = {";
    
    first = true;
    for (CSDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        CSDFchannel *c = *iter;
        
        if (!first)
            out << ", ";
        first = false;
                
        out << c->getInitialTokens();
    }

    out << "};" << endl;            
    
    out << "uint channelRate[" << g->nrChannels() << "][2][" << period << "] = {";
    
    first = true;
    for (CSDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        CSDFchannel *c = *iter;
        
        if (!first)
            out << ", ";
        first = false;
        
        out << "{";
        
        out << "{";
        for (uint i = 0; i < period; i++)
        {
            if (i != 0)
                out << ", ";
                
            out << c->getSrcPort()->getRate()[i];
        }
        out << "}";

        out << ", ";
        
        out << "{";
        for (uint i = 0; i < period; i++)
        {
            if (i != 0)
                out << ", ";
                
            out << c->getDstPort()->getRate()[i];
        }
        out << "}";
        
        out << "}";
    }

    out << "};" << endl;            
    
    out << "uint actorExecTime[" << g->nrActors() << "][" << period << "] = {";
    
    first = true;
    for (CSDFactorsIter iter = g->actorsBegin();
            iter != g->actorsEnd(); iter++)
    {
        TimedCSDFactor *a = (TimedCSDFactor*)(*iter);
        
        if (!first)
            out << ",";
        first = false;
        
        out << "{";
        
        for (uint i = 0; i < period; i++)
        {
            if (i != 0)
                out << ", ";
                
            out << a->getExecutionTime()[i];
        }
        
        out << "}";
    }

    out << "};" << endl;            
    out << endl;
    
    out << "TBufSize minSz[NR_CHANNELS] = ";
    printMinimalChannelSz(g, out);
    out << ";" << endl;

    out << "TBufSize minSzStep[NR_CHANNELS] = ";
    printMinimalChannelSzStep(g, out);
    out << ";" << endl;

    out.precision(30);
    out << "TBufSize lbDistributionSz = " << getLbDistributionSz(g) << ";" << endl;
    out << "TDtime maxThroughput = " << getMaxThroughput(g) << ";" << endl;
    out << "CId outputActor = " << getOutputActor(g) << ";" << endl;
    out << "TCnt outputActorRepCnt = " << getOutputActorRepCnt(g) << ";" << endl;
    out << "" << endl;
}

static
void printMiscFunctions(ostream &out)
{
    out << "// Performance counters" << endl;
    out << "unsigned long long maxNrStatesVisisted = 0, maxNrStatesStored = 0;" << endl;
    out << "unsigned long long nrDistributionsChecked = 0;" << endl;
    out << "unsigned long long nrStatesVisited, nrStatesStored;" << endl;
    out << "" << endl;
    out << "// Timer" << endl;
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
    out << "static" << endl;
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
    out << "typedef struct _StateActorFiring" << endl;
    out << "{" << endl;
    out << "    TTime execTime;" << endl;
    out << "    TCnt  seqPos;" << endl;
    out << "} StateActorFiring;" << endl;
    out << "" << endl;
    out << "bool operator==(const StateActorFiring &s1, const StateActorFiring &s2)" << endl;
    out << "{" << endl;
    out << "    if (s1.execTime != s2.execTime || s1.seqPos != s2.seqPos)" << endl;
    out << "        return false;" << endl;
    out << "        " << endl;
    out << "    return true;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "typedef struct _State" << endl;
    out << "{" << endl;
    out << "    list<StateActorFiring> actClk[NR_ACTORS];" << endl;
    out << "    TCnt actSeqPos[NR_ACTORS];" << endl;
    out << "    TBufSize ch[NR_CHANNELS];" << endl;
    out << "    TBufSize sp[NR_CHANNELS];" << endl;
    out << "    unsigned long glbClk;  " << endl;
    out << "} State;" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * printState ()" << endl;
    out << " * Print the state to the supplied stream." << endl;
    out << " */" << endl;
    out << "void printState(const State &s, ostream &out)" << endl;
    out << "{" << endl;
    out << "    out << \"### State ###\" << endl;" << endl;
    out << "" << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "    {" << endl;
    out << "        out << \"actClk[\" << i << \"] =\";" << endl;
    out << "        " << endl;
    out << "        for (list<StateActorFiring>::const_iterator iter = s.actClk[i].begin();" << endl;
    out << "                iter != s.actClk[i].end(); iter++)" << endl;
    out << "        {" << endl;
    out << "            out << \" (\" << (*iter).execTime << \", \";" << endl;
    out << "            out << (*iter).seqPos << \"),\";" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        out << endl;" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "    {" << endl;
    out << "        out << \"actSeqPos[\" << i << \"] = \" << s.actSeqPos[i] << endl;" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << endl;
    out << "    {" << endl;
    out << "        out << \"ch[\" << i << \"] = \" << s.ch[i] << endl;" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << endl;
    out << "    {" << endl;
    out << "        out << \"sp[\" << i << \"] = \" << s.sp[i] << endl;" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    out << \"glbClk = \" << s.glbClk << endl;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * clearState ()" << endl;
    out << " * The function sets the state to zero." << endl;
    out << " */" << endl;
    out << "void clearState(State &s)" << endl;
    out << "{" << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "    {" << endl;
    out << "        s.actClk[i].clear();" << endl;
    out << "        s.actSeqPos[i] = 0;" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << endl;
    out << "    {" << endl;
    out << "        s.ch[i] = 0;" << endl;
    out << "        s.sp[i] = 0;" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    s.glbClk = 0;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * equalStates ()" << endl;
    out << " * The function compares to states and returns true if they are equal." << endl;
    out << " */" << endl;
    out << "inline" << endl;
    out << "bool equalStates(const State &s1, const State &s2)" << endl;
    out << "{" << endl;
    out << "    if (s1.glbClk != s2.glbClk)" << endl;
    out << "        return false;" << endl;
    out << "    " << endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << endl;
    out << "    {" << endl;
    out << "        if (s1.ch[i] != s2.ch[i] || s1.sp[i] != s2.sp[i])" << endl;
    out << "            return false;" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "    {" << endl;
    out << "        if (s1.actClk[i] != s2.actClk[i])" << endl;
    out << "            return false;" << endl;
    out << "        " << endl;
    out << "        if (s1.actSeqPos[i] != s2.actSeqPos[i])" << endl;
    out << "            return false;" << endl;
    out << "    }" << endl;
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
    out << "    to.glbClk = from.glbClk;" << endl;
    out << "    " << endl;
    out << "    for (uint i = 0; i < NR_CHANNELS; i++)" << endl;
    out << "    {" << endl;
    out << "        to.ch[i] = from.ch[i];" << endl;
    out << "        to.sp[i] = from.sp[i];" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "    {" << endl;
    out << "        to.actClk[i] = from.actClk[i];" << endl;
    out << "        to.actSeqPos[i] = from.actSeqPos[i];" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/******************************************************************************" << endl;
    out << " * States" << endl;
    out << " *****************************************************************************/" << endl;
    out << "" << endl;
    out << "typedef list<State>         States;" << endl;
    out << "typedef States::iterator    StatesIter;" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * storedStates" << endl;
    out << " * List of visited states that are stored." << endl;
    out << " */" << endl;
    out << "static States storedStates;" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * storeState ()" << endl;
    out << " * The function stores the state s on whenever s is not already in the" << endl;
    out << " * list of storedStates. When s is stored, the function returns true. When the" << endl;
    out << " * state s is already in the list, the state s is not stored. The function" << endl;
    out << " * returns false. The function always sets the pos variable to the position" << endl;
    out << " * where the state s is in the list." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "bool storeState(State &s, StatesIter &pos)" << endl;
    out << "{" << endl;
    out << "    // Find state in the list of stored states" << endl;
    out << "    for (StatesIter iter = storedStates.begin();" << endl;
    out << "            iter != storedStates.end(); iter++)" << endl;
    out << "    {" << endl;
    out << "        State &x = *iter;" << endl;
    out << "        " << endl;
    out << "        // State s at position iter in the list?" << endl;
    out << "        if (equalStates(x, s))" << endl;
    out << "        {" << endl;
    out << "            pos = iter;" << endl;
    out << "            return false;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    // State not found, store it at the end of the list" << endl;
    out << "    storedStates.push_back(s);" << endl;
    out << "    " << endl;
    out << "    // Added state to the end of the list" << endl;
    out << "    pos = storedStates.end();" << endl;
    out << "    " << endl;
    out << "    return true;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * clearStoredStates ()" << endl;
    out << " * The function clears the list of stored states." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void clearStoredStates()" << endl;
    out << "{" << endl;
    out << "    storedStates.clear();" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printDependencies(ostream &out)
{
    out << "/******************************************************************************" << endl;
    out << " * Dependencies" << endl;
    out << " *****************************************************************************/" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * dfsVisitDependencies ()" << endl;
    out << " * The function performs a DFS on the abstract dependency graph from a node a to" << endl;
    out << " * find all cycles of which a is part. Channels on a cycle are when needed" << endl;
    out << " * marked to have a storage dependency." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void dfsVisitDependencies(uint a, int *color, int *pi," << endl;
    out << "        bool **abstractDepGraph, bool *dep)" << endl;
    out << "{" << endl;
    out << "    uint c, d;" << endl;
    out << "" << endl;
    out << "    // Mark node a as visited" << endl;
    out << "    color[a] = 1;" << endl;
    out << "    " << endl;
    out << "    // Visit all nodes reachable from a" << endl;
    out << "    for (uint b = 0; b < NR_ACTORS; b++)" << endl;
    out << "    {" << endl;
    out << "        // Node b reachable from a?" << endl;
    out << "        if (abstractDepGraph[a][b])" << endl;
    out << "        {" << endl;
    out << "            // Visited node b before?" << endl;
    out << "            if (color[b] == 1)" << endl;
    out << "            {" << endl;
    out << "                // Found a cycle in the graph containing node b" << endl;
    out << "                c = a;" << endl;
    out << "                d = b;           " << endl;
    out << "                do" << endl;
    out << "                {" << endl;
    out << "                    // All channels from d to c in the SDFG have " << endl;
    out << "                    // storage dependency" << endl;
    out << "                    for (uint ch = 0; ch < NR_CHANNELS; ch++)" << endl;
    out << "                    {" << endl;
    out << "                        CId srcId = channel[ch][SRC];" << endl;
    out << "                        CId dstId = channel[ch][DST];" << endl;
    out << "                        " << endl;
    out << "                        if (dstId == d && srcId == c)" << endl;
    out << "                            dep[ch] = true;" << endl;
    out << "                    }" << endl;
    out << "" << endl;
    out << "                    // Next" << endl;
    out << "                    d = c;" << endl;
    out << "                    c = pi[d];" << endl;
    out << "                } while (d != b);" << endl;
    out << "            }" << endl;
    out << "            else" << endl;
    out << "            {" << endl;
    out << "                // Visit node b" << endl;
    out << "                pi[b] = a;" << endl;
    out << "                dfsVisitDependencies(b, color, pi, abstractDepGraph, dep);" << endl;
    out << "            }" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Discovered all cycles which actor a is part of. Remove its edges to avoid" << endl;
    out << "    // re-discovering same cycles again." << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "    {" << endl;
    out << "        abstractDepGraph[i][a] = false;" << endl;
    out << "        abstractDepGraph[a][i] = false;" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Mark node a as not visited" << endl;
    out << "    color[a] = 0;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * findStorageDependencies ()" << endl;
    out << " * The function find all cycles in the abstract dependency graph. Any channel" << endl;
    out << " * modeling storage space which is part of a cycle is marked to have a storage" << endl;
    out << " * dependency." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void findStorageDependencies(bool **abstractDepGraph, bool *dep)" << endl;
    out << "{" << endl;
    out << "    int *color, *pi;" << endl;
    out << "    " << endl;
    out << "    // Initialize DFS data structures" << endl;
    out << "    color = new int [NR_ACTORS];" << endl;
    out << "    pi = new int [NR_ACTORS];" << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "        color[i] = 0;" << endl;
    out << "        " << endl;
    out << "    // Initialize storage dependencies" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "        dep[c] = false;" << endl;
    out << "    " << endl;
    out << "    // DFS from every node in the graph to find all cycles" << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "    {" << endl;
    out << "        pi[i] = i;" << endl;
    out << "        dfsVisitDependencies(i, color, pi, abstractDepGraph, dep);" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    // Cleanup" << endl;
    out << "    delete [] color;" << endl;
    out << "    delete [] pi;" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printCSDFGheader(ostream &out)
{
    out << "/******************************************************************************" << endl;
    out << " * SDF" << endl;
    out << " *****************************************************************************/" << endl;
    out << "" << endl;
    out << "#define CH(c)               currentState.ch[c]" << endl;
    out << "#define SP(c)               currentState.sp[c]  " << endl;
    out << "#define CH_TOKENS(c,n)      (CH(c) >= n)  " << endl;
    out << "#define CH_SPACE(c,n)       (SP(c) >= n)" << endl;
    out << "#define CONSUME(c,n)        CH(c) = CH(c) - n;" << endl;
    out << "#define PRODUCE(c,n)        CH(c) = CH(c) + n;" << endl;
    out << "#define CONSUME_SP(c,n)     SP(c) = SP(c) - n;" << endl;
    out << "#define PRODUCE_SP(c,n)     SP(c) = SP(c) + n;" << endl;
    out << "" << endl;
    out << "#define ACT_SEQ_POS(a)      currentState.actSeqPos[a]" << endl;
    out << "#define ADV_ACT_SEQ_POS(a)  ACT_SEQ_POS(a) = (ACT_SEQ_POS(a) + 1) % PERIOD;" << endl;
    out << "" << endl;
    out << "#define CH_TOKENS_PREV(c,n) (previousState.ch[c] >= n)  " << endl;
    out << "#define CH_SPACE_PREV(c,n)  (previousState.sp[c] >= n)" << endl;
    out << "" << endl;
    out << "static State currentState;  " << endl;
    out << "static State previousState;  " << endl;
    out << "" << endl;
    out << "/**  " << endl;
    out << " * computeThroughput ()  " << endl;
    out << " * The function calculates the throughput of the states on the cycle. Its  " << endl;
    out << " * value is equal to the average number of firings of an actor per time unit." << endl;
    out << " */  " << endl;
    out << "static inline  " << endl;
    out << "TDtime computeThroughput(const StatesIter cycleIter)  " << endl;
    out << "{  " << endl;
    out << "	int nr_fire = 0;" << endl;
    out << "	TDtime time = 0;  " << endl;
    out << "" << endl;
    out << "	// Check all state from stack till cycle complete  " << endl;
    out << "	for (StatesIter iter = cycleIter; iter != storedStates.end(); iter++)" << endl;
    out << "    {" << endl;
    out << "        State &s = *iter;" << endl;
    out << "        " << endl;
    out << "        // Number of states in cycle is equal to number of iterations " << endl;
    out << "        // in the period" << endl;
    out << "        nr_fire++;" << endl;
    out << "        " << endl;
    out << "	    // Time between previous state  " << endl;
    out << "	    time += s.glbClk;" << endl;
    out << "	}" << endl;
    out << "" << endl;
    out << "	return (TDtime)(nr_fire)/(time);  " << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * actorReadyToFire ()" << endl;
    out << " * The function returns true when the actor is ready to fire in state" << endl;
    out << " * s. Else it returns false." << endl;
    out << " */" << endl;
    out << "inline" << endl;
    out << "bool actorReadyToFire(const uint a)" << endl;
    out << "{" << endl;
    out << "    // Check all input ports for tokens and output ports" << endl;
    out << "    // for space" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "    {" << endl;
    out << "        // Actor is destination of the channel?" << endl;
    out << "        if (channel[c][DST] == a)" << endl;
    out << "        {" << endl;
    out << "            if (!CH_TOKENS(c, channelRate[c][DST][ACT_SEQ_POS(a)]))" << endl;
    out << "            {" << endl;
    out << "                return false;" << endl;
    out << "            }    " << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Actor is source of the channel" << endl;
    out << "        if (channel[c][SRC] == a)" << endl;
    out << "        {" << endl;
    out << "            if (!CH_SPACE(c, channelRate[c][SRC][ACT_SEQ_POS(a)]))" << endl;
    out << "            {" << endl;
    out << "                return false;" << endl;
    out << "            }" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    return true;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * startActorFiring ()" << endl;
    out << " * Start the actor firing. Remove tokens from all input channels and" << endl;
    out << " * reserve space on all output channels. Finally, add the actor firing" << endl;
    out << " * to the list of active actor firings and advance sequence position." << endl;
    out << " */" << endl;
    out << "inline" << endl;
    out << "void startActorFiring(const uint a)" << endl;
    out << "{" << endl;
    out << "    StateActorFiring actFiring;" << endl;
    out << "    bool addedFiring = false;" << endl;
    out << "" << endl;
    out << "    // Set properties of this actor firing" << endl;
    out << "    actFiring.execTime = actorExecTime[a][ACT_SEQ_POS(a)];" << endl;
    out << "    actFiring.seqPos = ACT_SEQ_POS(a);" << endl;
    out << "    " << endl;
    out << "    // Consume tokens from inputs and space for output tokens" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "    {" << endl;
    out << "        // Actor is destination of the channel?" << endl;
    out << "        if (channel[c][DST] == a)" << endl;
    out << "        {" << endl;
    out << "            CONSUME(c, channelRate[c][DST][ACT_SEQ_POS(a)]);" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Actor is source of the channel" << endl;
    out << "        if (channel[c][SRC] == a)" << endl;
    out << "        {" << endl;
    out << "            CONSUME_SP(c, channelRate[c][SRC][ACT_SEQ_POS(a)]);" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Add actor firing to the list of active firings of this actor" << endl;
    out << "    for (list<StateActorFiring>::iterator iter = currentState.actClk[a].begin();" << endl;
    out << "                iter != currentState.actClk[a].end(); iter++)" << endl;
    out << "    {" << endl;
    out << "        if ((*iter).execTime >= actFiring.execTime)" << endl;
    out << "        {" << endl;
    out << "            currentState.actClk[a].insert(iter, actFiring);" << endl;
    out << "            addedFiring = true;" << endl;
    out << "            break;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "    if (!addedFiring)" << endl;
    out << "    {" << endl;
    out << "        // Not yet inserted, put it at the end" << endl;
    out << "        currentState.actClk[a].push_back(actFiring);" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Advance the sequence position of the actor" << endl;
    out << "    ADV_ACT_SEQ_POS(a);" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * actorReadyToEnd ()" << endl;
    out << " * The function returns true when the actor is ready to end its firing. Else" << endl;
    out << " * the function returns false." << endl;
    out << " */" << endl;
    out << "inline" << endl;
    out << "bool actorReadyToEnd(const uint a)" << endl;
    out << "{" << endl;
    out << "    // Actor currently not firing?" << endl;
    out << "    if (currentState.actClk[a].empty())" << endl;
    out << "        return false;" << endl;
    out << "    " << endl;
    out << "    // First actor firing in sorted list has execution time left?" << endl;
    out << "    if (currentState.actClk[a].front().execTime != 0)" << endl;
    out << "        return false;" << endl;
    out << "    " << endl;
    out << "    return true;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * endActorFiring ()" << endl;
    out << " * Produce tokens on all output channels and release space on all input" << endl;
    out << " * channels. Finally, remove the actor firing from the list of active" << endl;
    out << " * firings." << endl;
    out << " */" << endl;
    out << "inline" << endl;
    out << "void endActorFiring(const uint a)" << endl;
    out << "{" << endl;
    out << "    StateActorFiring &actFiring = currentState.actClk[a].front();" << endl;
    out << "   " << endl;
    out << "    // Produce tokens on outputs and space on inputs" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "    {" << endl;
    out << "        // Actor is source of the channel?" << endl;
    out << "        if (channel[c][SRC] == a)" << endl;
    out << "        {" << endl;
    out << "            PRODUCE(c, channelRate[c][SRC][actFiring.seqPos]);" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Actor is destination of the channel?" << endl;
    out << "        if (channel[c][DST] == a)" << endl;
    out << "        {" << endl;
    out << "            PRODUCE_SP(c, channelRate[c][DST][actFiring.seqPos]);" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Remove the firing from the list of active actor firings" << endl;
    out << "    currentState.actClk[a].pop_front();" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * clockStep ()" << endl;
    out << " * The function progresses time till the first end of firing transition" << endl;
    out << " * becomes enabled. The time step is returned. In case of deadlock, the" << endl;
    out << " * time step is equal to UINT_MAX." << endl;
    out << " */" << endl;
    out << "inline" << endl;
    out << "TTime clockStep()" << endl;
    out << "{" << endl;
    out << "    TTime step = UINT_MAX;" << endl;
    out << "    " << endl;
    out << "    // Find maximal time progress" << endl;
    out << "    for (uint a = 0; a < NR_ACTORS; a++)" << endl;
    out << "    {" << endl;
    out << "        if (!currentState.actClk[a].empty())" << endl;
    out << "        {" << endl;
    out << "            TTime actClk = currentState.actClk[a].front().execTime;" << endl;
    out << "" << endl;
    out << "            if (step > actClk)" << endl;
    out << "                step = actClk;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Still actors ready to end their firing?" << endl;
    out << "    if (step == 0)" << endl;
    out << "        return 0;" << endl;
    out << "" << endl;
    out << "	// Check for progress (i.e. no deadlock) " << endl;
    out << "	if (step == UINT_MAX)" << endl;
    out << "        return UINT_MAX;" << endl;
    out << "" << endl;
    out << "    // Lower remaining execution time actors" << endl;
    out << "    for (uint a = 0; a < NR_ACTORS; a++)" << endl;
    out << "    {" << endl;
    out << "        for (list<StateActorFiring>::iterator" << endl;
    out << "                iter = currentState.actClk[a].begin();" << endl;
    out << "                    iter != currentState.actClk[a].end(); iter++)" << endl;
    out << "        {" << endl;
    out << "            StateActorFiring &actFiring = *iter;" << endl;
    out << "            " << endl;
    out << "            // Lower remaining execution time of the actor firing" << endl;
    out << "            actFiring.execTime -= step;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Advance the global clock  " << endl;
    out << "    currentState.glbClk += step;" << endl;
    out << "    " << endl;
    out << "    return step;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * findCausalDependencies ()" << endl;
    out << " * The function tracks all causal dependencies in the actor firing of actor a." << endl;
    out << " * Any causal dependency that is found is added to the abstract dependency" << endl;
    out << " * graph." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void findCausalDependencies(const uint a, bool **abstractDepGraph)" << endl;
    out << "{" << endl;
    out << "    // Check all input ports for tokens and output ports for space" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "    {" << endl;
    out << "        // Actor is destination of the channel?" << endl;
    out << "        if (channel[c][DST] == a)" << endl;
    out << "        {" << endl;
    out << "            // Not enough tokens in the previous state?" << endl;
    out << "            if (!CH_TOKENS_PREV(c, channelRate[c][DST][ACT_SEQ_POS(a)]))" << endl;
    out << "            {" << endl;
    out << "                abstractDepGraph[channel[c][DST]][channel[c][SRC]] = true;" << endl;
    out << "            }    " << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Actor is source of the channel" << endl;
    out << "        if (channel[c][SRC] == a)" << endl;
    out << "        {" << endl;
    out << "            // Not enough space in the previous state?" << endl;
    out << "            if (!CH_SPACE_PREV(c, channelRate[c][SRC][ACT_SEQ_POS(a)]))" << endl;
    out << "            {" << endl;
    out << "                abstractDepGraph[channel[c][SRC]][channel[c][DST]] = true;" << endl;
    out << "            }" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printAnalyzePeriodicPhase(ostream &out)
{
    out << "/**" << endl;
    out << " * analyzePeriodicPhase ()" << endl;
    out << " * Analyze the periodic phase of the schedule to find all blocked channels. This" << endl;
    out << " * is done using the abstract dependency graph." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void analyzePeriodicPhase(const TBufSize *sp, bool *dep)" << endl;
    out << "{" << endl;
    out << "    bool outputActorFound = false;" << endl;
    out << "    bool **abstractDepGraph;" << endl;
    out << "    State periodicState;" << endl;
    out << "    TTime clkStep;" << endl;
    out << "    int repCnt;  " << endl;
    out << "" << endl;
    out << "    // Current state is a periodic state" << endl;
    out << "    copyState(periodicState, currentState);" << endl;
    out << "" << endl;
    out << "    // Abstract dependency graph" << endl;
    out << "    abstractDepGraph = new bool* [NR_ACTORS];" << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "    {" << endl;
    out << "        abstractDepGraph[i] = new bool [NR_ACTORS];" << endl;
    out << "        for (uint j = 0; j < NR_ACTORS; j++)" << endl;
    out << "            abstractDepGraph[i][j] = false;" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Start new iteration of the periodic phase" << endl;
    out << "	currentState.glbClk = 0;" << endl;
    out << "    " << endl;
    out << "    // Still need to complete the last firing of the output actor" << endl;
    out << "    // before period really ends" << endl;
    out << "    repCnt = -1;" << endl;
    out << "" << endl;
    out << "    // Complete the remaining actor firings" << endl;
    out << "    for (uint a = outputActor; a < NR_ACTORS; a++)" << endl;
    out << "    {" << endl;
    out << "        while (actorReadyToEnd(a))  " << endl;
    out << "        {" << endl;
    out << "            if (a == outputActor)" << endl;
    out << "            {" << endl;
    out << "                repCnt++;" << endl;
    out << "                if (repCnt == outputActorRepCnt)" << endl;
    out << "                {" << endl;
    out << "                    currentState.glbClk = 0;" << endl;
    out << "                    repCnt = 0;" << endl;
    out << "                }" << endl;
    out << "            }" << endl;
    out << "" << endl;
    out << "            endActorFiring(a);" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Fire the actors" << endl;
    out << "    while (true)" << endl;
    out << "    {" << endl;
    out << "        // Start actor firings" << endl;
    out << "        for (uint a = 0; a < NR_ACTORS; a++)" << endl;
    out << "        {" << endl;
    out << "            // Ready to fire actor a?" << endl;
    out << "            while (actorReadyToFire(a))" << endl;
    out << "            {" << endl;
    out << "                // Track causal dependencies on firing of actor a" << endl;
    out << "                findCausalDependencies(a, abstractDepGraph);" << endl;
    out << "                " << endl;
    out << "                // Fire actor a" << endl;
    out << "                startActorFiring(a);" << endl;
    out << "            }" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Performance counter" << endl;
    out << "        nrStatesVisited++; " << endl;
    out << "" << endl;
    out << "        // Clock step" << endl;
    out << "        clkStep = clockStep();" << endl;
    out << "" << endl;
    out << "        // Store partial state to check for progress" << endl;
    out << "        for (uint i = 0; i < NR_CHANNELS; i++)" << endl;
    out << "        {" << endl;
    out << "            previousState.ch[i] = currentState.ch[i];" << endl;
    out << "            previousState.sp[i] = currentState.sp[i];" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Finish actor firings  " << endl;
    out << "        for (uint a = 0; a < NR_ACTORS; a++)" << endl;
    out << "        {" << endl;
    out << "            while (actorReadyToEnd(a))  " << endl;
    out << "            {" << endl;
    out << "                if (outputActor == a)" << endl;
    out << "                {" << endl;
    out << "                    repCnt++;" << endl;
    out << "                    if (repCnt == outputActorRepCnt)  " << endl;
    out << "                    { " << endl;
    out << "                        // Found periodic state" << endl;
    out << "                        if (equalStates(currentState, periodicState))" << endl;
    out << "                        {" << endl;
    out << "                            // Cycles in the dependency graph indicate storage" << endl;
    out << "                            // dependencies" << endl;
    out << "                            findStorageDependencies(abstractDepGraph, dep);" << endl;
    out << "" << endl;
    out << "                            // Cleanup" << endl;
    out << "                            for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "                                delete [] abstractDepGraph[i];" << endl;
    out << "                            delete [] abstractDepGraph;" << endl;
    out << "" << endl;
    out << "                            // Done" << endl;
    out << "                            return;" << endl;
    out << "                        }" << endl;
    out << "	                    currentState.glbClk = 0;" << endl;
    out << "                        repCnt = 0;  " << endl;
    out << "                    }  " << endl;
    out << "                }" << endl;
    out << "" << endl;
    out << "                // End the actor firing" << endl;
    out << "                endActorFiring(a);" << endl;
    out << "            }  " << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static 
void printAnalyzeDeadlock(ostream &out)
{
    out << "/**" << endl;
    out << " * analyzeDeadlock ()" << endl;
    out << " * Analyze the deadlock in the schedule." << endl;
    out << " */" << endl;
    out << "static " << endl;
    out << "void analyzeDeadlock(const TBufSize *sp, bool *dep)" << endl;
    out << "{" << endl;
    out << "    bool **abstractDepGraph;" << endl;
    out << "    " << endl;
    out << "    // Abstract dependency graph" << endl;
    out << "    abstractDepGraph = new bool* [NR_ACTORS];" << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "    {" << endl;
    out << "        abstractDepGraph[i] = new bool [NR_ACTORS];" << endl;
    out << "        for (uint j = 0; j < NR_ACTORS; j++)" << endl;
    out << "            abstractDepGraph[i][j] = false;" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Check number of tokens on every channel in the graph" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "    {" << endl;
    out << "        CId srcActor = channel[c][SRC];" << endl;
    out << "        CId dstActor = channel[c][DST];" << endl;
    out << "        " << endl;
    out << "        // Insufficient tokens to fire destination actor" << endl;
    out << "        if (!CH_TOKENS(c, channelRate[c][DST][ACT_SEQ_POS(dstActor)]))" << endl;
    out << "        {" << endl;
    out << "            abstractDepGraph[dstActor][srcActor] = true;" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Insufficient space to fire source actor" << endl;
    out << "        if (!CH_SPACE(c, channelRate[c][SRC][ACT_SEQ_POS(srcActor)]))" << endl;
    out << "        {" << endl;
    out << "            abstractDepGraph[srcActor][dstActor] = true;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Cycles in the dependency graph indicate storage dependencies" << endl;
    out << "    findStorageDependencies(abstractDepGraph, dep);" << endl;
    out << "    " << endl;
    out << "    // Cleanup" << endl;
    out << "    for (uint i = 0; i < NR_ACTORS; i++)" << endl;
    out << "        delete [] abstractDepGraph[i];" << endl;
    out << "    delete [] abstractDepGraph;" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printExecCSDFG(ostream &out)
{
    out << "/**  " << endl;
    out << " * execCSDFgraph()  " << endl;
    out << " * Execute the CSDF graph till a deadlock is found or a recurrent state.  " << endl;
    out << " * The throughput is returned.  " << endl;
    out << " */  " << endl;
    out << "static" << endl;
    out << "TDtime execCSDFgraph(const TBufSize *sp, bool *dep)  " << endl;
    out << "{" << endl;
    out << "    StatesIter recurrentState;" << endl;
    out << "    TTime clkStep;" << endl;
    out << "    int repCnt = 0;  " << endl;
    out << "" << endl;
    out << "    // Create initial state" << endl;
    out << "    clearState(currentState);  " << endl;
    out << "    clearState(previousState);  " << endl;
    out << "" << endl;
    out << "    // Initial tokens and space" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "    {" << endl;
    out << "        // Not enough space for initial tokens?" << endl;
    out << "        if (sp[c] < channelInitialTokens[c])" << endl;
    out << "        {" << endl;
    out << "            dep[c] = true;" << endl;
    out << "            return 0;" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        CH(c) = channelInitialTokens[c];" << endl;
    out << "        SP(c) = sp[c] - channelInitialTokens[c];" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    // Fire the actors  " << endl;
    out << "    while (true)  " << endl;
    out << "    {" << endl;
    out << "        // Performance counter" << endl;
    out << "        nrStatesVisited++; " << endl;
    out << "" << endl;
    out << "        // Store partial state to check for progress" << endl;
    out << "        for (uint i = 0; i < NR_CHANNELS; i++)" << endl;
    out << "        {" << endl;
    out << "            previousState.ch[i] = currentState.ch[i];" << endl;
    out << "            previousState.sp[i] = currentState.sp[i];" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Finish actor firings  " << endl;
    out << "        for (uint a = 0; a < NR_ACTORS; a++)" << endl;
    out << "        {" << endl;
    out << "            while (actorReadyToEnd(a))  " << endl;
    out << "            {" << endl;
    out << "                if (outputActor == a)" << endl;
    out << "                {" << endl;
    out << "                    repCnt++;" << endl;
    out << "                    if (repCnt == outputActorRepCnt)  " << endl;
    out << "                    { " << endl;
    out << "                        // Add state to hash of visited states  " << endl;
    out << "                        if (!storeState(currentState, recurrentState))" << endl;
    out << "                        {" << endl;
    out << "                            // Find storage dependencies in periodic phase" << endl;
    out << "                            analyzePeriodicPhase(sp, dep);" << endl;
    out << "" << endl;
    out << "                            return computeThroughput(recurrentState);" << endl;
    out << "                        }" << endl;
    out << "	                    currentState.glbClk = 0;" << endl;
    out << "                        repCnt = 0;" << endl;
    out << "                        " << endl;
    out << "                        // Performance counter" << endl;
    out << "                        nrStatesStored++;" << endl;
    out << "                    }  " << endl;
    out << "                }" << endl;
    out << "" << endl;
    out << "                // End the actor firing" << endl;
    out << "                endActorFiring(a);" << endl;
    out << "            }  " << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Start actor firings  " << endl;
    out << "        for (uint a = 0; a < NR_ACTORS; a++)" << endl;
    out << "        {" << endl;
    out << "            // Ready to fire actor a?" << endl;
    out << "            while (actorReadyToFire(a))" << endl;
    out << "            {" << endl;
    out << "                // Fire actor a" << endl;
    out << "                startActorFiring(a);" << endl;
    out << "            }" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Clock step" << endl;
    out << "        clkStep = clockStep();" << endl;
    out << "" << endl;
    out << "        // Deadlocked?" << endl;
    out << "        if (clkStep == UINT_MAX)" << endl;
    out << "        {" << endl;
    out << "            // Find cause of deadlock" << endl;
    out << "            analyzeDeadlock(sp, dep);" << endl;
    out << "            return 0;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    return 0;" << endl;
    out << "}  " << endl;
    out << "" << endl;
}

static
void printCSDFGexecution(ostream &out)
{
    printCSDFGheader(out);
    printAnalyzePeriodicPhase(out);
    printAnalyzeDeadlock(out);
    printExecCSDFG(out);
}

static
void printDistribution(TimedCSDFgraph *g, ostream &out)
{
    out << "/*******************************************************************************" << endl;
    out << " * Distributions" << endl;
    out << " ******************************************************************************/" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * Distribution" << endl;
    out << " * A storage distribution." << endl;
    out << " */" << endl;
    out << "typedef struct _Distribution" << endl;
    out << "{" << endl;
    out << "    TBufSize sz;" << endl;
    out << "    TBufSize sp[NR_CHANNELS];" << endl;
    out << "    TDtime thr;" << endl;
    out << "    bool dep[NR_CHANNELS];" << endl;
    out << "    struct _Distribution *prev;" << endl;
    out << "    struct _Distribution *next;" << endl;
    out << "} Distribution;" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * DistributionSet" << endl;
    out << " * A container for a linked-list of storage distributions with the same" << endl;
    out << " * size. The container can also be used to build a linked-list of" << endl;
    out << " * distributions of different size." << endl;
    out << " */" << endl;
    out << "typedef struct _DistributionSet" << endl;
    out << "{" << endl;
    out << "    TDtime thr;" << endl;
    out << "    TBufSize sz;" << endl;
    out << "    Distribution *distributions;" << endl;
    out << "    struct _DistributionSet *prev;" << endl;
    out << "    struct _DistributionSet *next;" << endl;
    out << "} DistributionSet;" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * minStorageDistributions" << endl;
    out << " * List of all minimal storage distributions." << endl;
    out << " */" << endl;
    out << "static DistributionSet *minStorageDistributions = NULL;" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * newDistribution ()" << endl;
    out << " * Allocate memory for a new storage distribution." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "Distribution *newDistribution()" << endl;
    out << "{" << endl;
    out << "    return new Distribution;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * deleteDistribution ()" << endl;
    out << " * Deallocate memory for a storage distribution." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void deleteDistribution(Distribution *d)" << endl;
    out << "{" << endl;
    out << "    delete d;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * execDistribution ()" << endl;
    out << " * Compute throughput and storage dependencies of the given storage" << endl;
    out << " * distribution." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void execDistribution(Distribution *d)" << endl;
    out << "{" << endl;
    out << "    // Clear list of stored states" << endl;
    out << "    clearStoredStates();" << endl;
    out << "" << endl;
    out << "    // Initialize performance counters" << endl;
    out << "    nrStatesVisited = 0;" << endl;
    out << "    nrStatesStored = 0;" << endl;
    out << "    nrDistributionsChecked++;" << endl;
    out << "    " << endl;
    out << "    // Initialize blocking channels" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "        d->dep[c] = false;" << endl;
    out << "" << endl;
    out << "    // Execute the CSDF graph to find its output interval" << endl;
    out << "    d->thr = execCSDFgraph(d->sp, d->dep);" << endl;
    out << "" << endl;
    out << "    // Update performance counters" << endl;
    out << "    if (maxNrStatesVisisted < nrStatesVisited)" << endl;
    out << "        maxNrStatesVisisted = nrStatesVisited;" << endl;
    out << "    if (maxNrStatesStored < nrStatesStored)" << endl;
    out << "        maxNrStatesStored = nrStatesStored;" << endl;
    out << "" << endl;
    out << "    //cerr << d->sz << \" \" << d->thr << endl;" << endl;
    out << "    //for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "    //    cerr << d->sp[c] << \" \" << d->dep[c] << endl;" << endl;
    out << "    //cerr << endl;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * minimizeMinStorageDistributions ()" << endl;
    out << " * The function removes all storage distributions within the supplied" << endl;
    out << " * set of storage distributions which are non-minimal. All storage distributions" << endl;
    out << " * within the set should have the same size." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void minimizeStorageDistributionsSet(DistributionSet *ds)" << endl;
    out << "{" << endl;
    out << "    Distribution *d, *t;" << endl;
    out << "    " << endl;
    out << "    // Maximal throughput with current distribution size equal to previous" << endl;
    out << "    // throughput with previous (smaller) distribution size" << endl;
    out << "    if (ds->prev != NULL && ds->prev->thr == ds->thr)" << endl;
    out << "    {" << endl;
    out << "        // No minimal storage distributions exist in this list" << endl;
    out << "        // Iterate over the list of storage distributions    " << endl;
    out << "        d = ds->distributions;" << endl;
    out << "        while (d != NULL)" << endl;
    out << "        {" << endl;
    out << "            // Temporary reference to next element in list" << endl;
    out << "            t = d->next;" << endl;
    out << "" << endl;
    out << "            // Cleanup d" << endl;
    out << "            deleteDistribution(d);" << endl;
    out << "" << endl;
    out << "            // Next" << endl;
    out << "            d = t;" << endl;
    out << "        }" << endl;
    out << "        " << endl;
    out << "        // No distributions left" << endl;
    out << "        ds->distributions = NULL;" << endl;
    out << "    }" << endl;
    out << "    else" << endl;
    out << "    {" << endl;
    out << "        // Iterate over the list of storage distributions    " << endl;
    out << "        d = ds->distributions;" << endl;
    out << "        while (d != NULL)" << endl;
    out << "        {" << endl;
    out << "            // Throughput of distribution smaller then maximum throughput with" << endl;
    out << "            // same distribution size?" << endl;
    out << "            if (d->thr < ds->thr)" << endl;
    out << "            {" << endl;
    out << "                // Remove d from linked list" << endl;
    out << "                if (d->prev != NULL)" << endl;
    out << "                    d->prev->next = d->next;" << endl;
    out << "                else" << endl;
    out << "                    ds->distributions = d->next;" << endl;
    out << "                if (d->next != NULL)" << endl;
    out << "                    d->next->prev = d->prev;" << endl;
    out << "" << endl;
    out << "                // Temporary reference to next element in list" << endl;
    out << "                t = d->next;" << endl;
    out << "" << endl;
    out << "                // Cleanup d" << endl;
    out << "                deleteDistribution(d);" << endl;
    out << "" << endl;
    out << "                // Next" << endl;
    out << "                d = t;" << endl;
    out << "            }" << endl;
    out << "            else" << endl;
    out << "            {" << endl;
    out << "                // Next" << endl;
    out << "                d = d->next;" << endl;
    out << "            }" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * addDistributionToChecklist ()" << endl;
    out << " * The function add the storage distribution 'd' to the list of" << endl;
    out << " * storage distributions which must still be checked. This list is" << endl;
    out << " * ordered wrt to the size of the storage distribution. A storage" << endl;
    out << " * distribution is only added if it is not already contained in the" << endl;
    out << " * list. When the distribution is added to the list, the function returns" << endl;
    out << " * 'true', else the function returns 'false'." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "bool addDistributionToChecklist(Distribution *d)" << endl;
    out << "{" << endl;
    out << "    DistributionSet *ds, *dsNew;" << endl;
    out << "    Distribution *di;" << endl;
    out << "    bool equalDistr;" << endl;
    out << "    " << endl;
    out << "    // First distribution ever added?" << endl;
    out << "    if (minStorageDistributions == NULL)" << endl;
    out << "    {" << endl;
    out << "        // Create new set of storage distributions" << endl;
    out << "        dsNew = new DistributionSet;" << endl;
    out << "        dsNew->sz = d->sz;" << endl;
    out << "        dsNew->thr = 0;" << endl;
    out << "        dsNew->distributions = d;" << endl;
    out << "        dsNew->next = NULL;" << endl;
    out << "        dsNew->prev = NULL;" << endl;
    out << "        " << endl;
    out << "        // Set is first in list to be checked" << endl;
    out << "        minStorageDistributions = dsNew;" << endl;
    out << "        " << endl;
    out << "        // Done" << endl;
    out << "        return true;" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    // Find set of distributions with the same size" << endl;
    out << "    ds = minStorageDistributions;" << endl;
    out << "    while (ds->next != NULL && ds->next->sz <= d->sz)" << endl;
    out << "    {" << endl;
    out << "        ds = ds->next;" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    // Set of storage distribution with same size as d exists?" << endl;
    out << "    if (ds->sz == d->sz)" << endl;
    out << "    {" << endl;
    out << "" << endl;
    out << "        // Check that distribution d does not exist in the set" << endl;
    out << "        di = ds->distributions;" << endl;
    out << "        while (di != NULL)" << endl;
    out << "        {" << endl;
    out << "            equalDistr = true;" << endl;
    out << "" << endl;
    out << "            // All channels have same storage space allocation?" << endl;
    out << "            for (uint c = 0; equalDistr && c < NR_CHANNELS; c++)" << endl;
    out << "            {" << endl;
    out << "                if (di->sp[c] != d->sp[c])" << endl;
    out << "                    equalDistr = false;" << endl;
    out << "            }" << endl;
    out << "" << endl;
    out << "            // Found equal distribution" << endl;
    out << "            if (equalDistr)" << endl;
    out << "                return false;" << endl;
    out << "" << endl;
    out << "            // Next" << endl;
    out << "            di = di->next;" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Distribution 'd' not yet in the set, so let's add it" << endl;
    out << "        ds->distributions->prev = d;" << endl;
    out << "        d->next = ds->distributions;" << endl;
    out << "        ds->distributions = d;" << endl;
    out << "    }" << endl;
    out << "    else if (ds->next == NULL)" << endl;
    out << "    {" << endl;
    out << "        // No set of distribution in the list with same or larger size?	" << endl;
    out << "        " << endl;
    out << "        // Create new set of storage distributions" << endl;
    out << "        dsNew = new DistributionSet;" << endl;
    out << "        dsNew->sz = d->sz;" << endl;
    out << "        dsNew->thr = 0;" << endl;
    out << "        dsNew->distributions = d;" << endl;
    out << "        dsNew->next = NULL;" << endl;
    out << "        dsNew->prev = ds;" << endl;
    out << "        " << endl;
    out << "        // Add set to the list of sets" << endl;
    out << "        ds->next = dsNew;" << endl;
    out << "    }    " << endl;
    out << "    else" << endl;
    out << "    {" << endl;
    out << "        // In this case 'ds->next' has a size larger then 'd' and 'ds' has" << endl;
    out << "        // a size smaller then 'd'. Insert a new set of storage" << endl;
    out << "        // distributions between 'ds' and 'ds->next' containing storage" << endl;
    out << "        // distribution 'd'." << endl;
    out << "" << endl;
    out << "        // Create new set of storage distributions" << endl;
    out << "        dsNew = new DistributionSet;" << endl;
    out << "        dsNew->sz = d->sz;" << endl;
    out << "        dsNew->thr = 0;" << endl;
    out << "        dsNew->distributions = d;" << endl;
    out << "        dsNew->next = ds->next;" << endl;
    out << "        dsNew->prev = ds;" << endl;
    out << "" << endl;
    out << "        // Add set to the list of sets" << endl;
    out << "        ds->next->prev = dsNew;" << endl;
    out << "        ds->next = dsNew;" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    // Done" << endl;
    out << "    return true;" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * exploreDistribution ()" << endl;
    out << " * The function computes the throughput of the storage distribution" << endl;
    out << " * 'd' and adds new storage distributions to the list of distributions" << endl;
    out << " * which must be checked based on the storage dependencies found in d." << endl;
    out << " * The function also updates the maximal throughput of the set of" << endl;
    out << " * storage distributions when needed." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void exploreDistribution(DistributionSet *ds, Distribution *d)" << endl;
    out << "{" << endl;
    out << "    Distribution *dNew;" << endl;
    out << "    " << endl;
    out << "    // Compute throughput and storage dependencies" << endl;
    out << "    execDistribution(d);" << endl;
    out << "    " << endl;
    out << "    // Throughput of d larger then current maximum of the set" << endl;
    out << "    if (d->thr > ds->thr)" << endl;
    out << "        ds->thr = d->thr;" << endl;
    out << "    " << endl;
    out << "    // Create new storage distributions for every channel which" << endl;
    out << "    // has a storage dependency in d" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "    {" << endl;
    out << "        // Channel c has storage dependency?" << endl;
    out << "        if (d->dep[c])" << endl;
    out << "        {" << endl;
    out << "            // Do not enlarge the channel if its a self-edge" << endl;
    out << "            if (channel[c][SRC] == channel[c][DST])" << endl;
    out << "                continue;" << endl;
    out << "                " << endl;
    out << "            // Create new storage distribution with channel c enlarged" << endl;
    out << "            dNew = newDistribution();" << endl;
    out << "            dNew->sz = d->sz + minSzStep[c];" << endl;
    out << "            dNew->thr = 0;" << endl;
    out << "            for (uint i = 0; i < NR_CHANNELS; i++)" << endl;
    out << "            {" << endl;
    out << "                dNew->sp[i] = d->sp[i];" << endl;
    out << "                if (i == c)" << endl;
    out << "                    dNew->sp[i] += minSzStep[c];" << endl;
    out << "            }" << endl;
    out << "            dNew->next = NULL;" << endl;
    out << "            dNew->prev = NULL;" << endl;
    out << "            " << endl;
    out << "            // Add storage distribution to set of distributions to be checked" << endl;
    out << "            if (!addDistributionToChecklist(dNew))" << endl;
    out << "            {" << endl;
    out << "                // Distribution already in check list" << endl;
    out << "                deleteDistribution(dNew);" << endl;
    out << "            }" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * exploreDistributionSet ()" << endl;
    out << " * Explore all distributions within the set and remove all non-minimal" << endl;
    out << " * distributions from it." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void exploreDistributionSet(DistributionSet *ds)" << endl;
    out << "{" << endl;
    out << "    Distribution *d;" << endl;
    out << "" << endl;
    out << "    // Explore all storage distributions contained in the set    " << endl;
    out << "    d = ds->distributions;" << endl;
    out << "    while (d != NULL)" << endl;
    out << "    {" << endl;
    out << "        // Explore distribution d" << endl;
    out << "        exploreDistribution(ds, d);" << endl;
    out << "        " << endl;
    out << "        // Next" << endl;
    out << "        d = d->next;" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    // Remove all non-minimal storage distributions from the set" << endl;
    out << "    minimizeStorageDistributionsSet(ds);" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * findMinimalStorageDistributions ()" << endl;
    out << " * Explore the throughput/storage-size trade-off space till either" << endl;
    out << " * all minimal storage distributions are found or the throughput bound is" << endl;
    out << " * reached." << endl;
    out << " */" << endl;
    out << "static" << endl;
    out << "void findMinimalStorageDistributions(const double thrBound)" << endl;
    out << "{" << endl;
    out << "    Distribution *d, *t;" << endl;
    out << "    DistributionSet *ds, *dt;" << endl;
    out << "    " << endl;
    out << "    // Construct storage distribution with lower bound storage space" << endl;
    out << "    d = newDistribution();" << endl;
    out << "    d->thr = 0;" << endl;
    out << "    d->sz = lbDistributionSz;" << endl;
    out << "    for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "        d->sp[c] = minSz[c];" << endl;
    out << "    d->next = NULL;" << endl;
    out << "    d->prev = NULL;" << endl;
    out << "    " << endl;
    out << "    // Add distribution to set of distributions which must be checked" << endl;
    out << "    addDistributionToChecklist(d);" << endl;
    out << "    " << endl;
    out << "    // Check sets of storage distributions till no distributions left to check," << endl;
    out << "    // or throughput bound exceeded, or maximal throughput reached" << endl;
    out << "    ds = minStorageDistributions;" << endl;
    out << "    while (ds != NULL)" << endl;
    out << "    {" << endl;
    out << "        // Explore all distributions with size 'ds->sz'" << endl;
    out << "        exploreDistributionSet(ds);" << endl;
    out << "" << endl;
    out << "        // Reached maximum throughput or exceed thrBound" << endl;
    out << "        if (ds->thr >= thrBound || ds->thr >= maxThroughput)" << endl;
    out << "            break;" << endl;
    out << "" << endl;
    out << "        // Temporary pointer to set" << endl;
    out << "        dt = ds;" << endl;
    out << "" << endl;
    out << "        // Next" << endl;
    out << "        ds = ds->next;" << endl;
    out << "" << endl;
    out << "        // No distributions left with the given distribution size?" << endl;
    out << "        if (dt->distributions == NULL)" << endl;
    out << "        {" << endl;
    out << "            // Remove distr from linked list" << endl;
    out << "            if (dt->prev != NULL)" << endl;
    out << "                dt->prev->next = dt->next;" << endl;
    out << "            else" << endl;
    out << "                minStorageDistributions = dt->next;" << endl;
    out << "            if (dt->next != NULL)" << endl;
    out << "                dt->next->prev = dt->prev;" << endl;
    out << "" << endl;
    out << "            // Cleanup dt" << endl;
    out << "            delete dt;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "   " << endl;
    out << "    // Unexplored distributions left?" << endl;
    out << "    if (ds != NULL && ds->next != NULL)" << endl;
    out << "    {" << endl;
    out << "        // Pointer to first set which must be removed" << endl;
    out << "        ds = ds->next;" << endl;
    out << "        " << endl;
    out << "        // Mark previous set as last set in list of minimal storage distr" << endl;
    out << "        ds->prev->next = NULL;" << endl;
    out << "        " << endl;
    out << "        // Remove all unexplored distributions (and sets)" << endl;
    out << "        while (ds != NULL)" << endl;
    out << "        {" << endl;
    out << "            // Remove all distributions within the set ds" << endl;
    out << "            d = ds->distributions;" << endl;
    out << "            while (d != NULL)" << endl;
    out << "            {" << endl;
    out << "                t = d->next;" << endl;
    out << "                deleteDistribution(d);" << endl;
    out << "                d = t;" << endl;
    out << "            }" << endl;
    out << "" << endl;
    out << "            // Temporary pointer to set" << endl;
    out << "            dt = ds;" << endl;
    out << "            " << endl;
    out << "            // Next" << endl;
    out << "            ds = ds->next;" << endl;
    out << "" << endl;
    out << "            // Cleanup dt" << endl;
    out << "            delete dt;" << endl;
    out << "        }" << endl;
    out << "    }" << endl;
    out << "    " << endl;
    out << "    // Lower bound on storage space (which is used in the beginning) is not a" << endl;
    out << "    // minimal storage distribution if it deadlocks. The distribution <0,...,0>" << endl;
    out << "    // is the actual minimal storage distribution for this throughput" << endl;
    out << "    if (minStorageDistributions->thr == 0)" << endl;
    out << "    {" << endl;
    out << "        minStorageDistributions->sz = 0;" << endl;
    out << "        minStorageDistributions->distributions->sz = 0;" << endl;
    out << "        for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "            minStorageDistributions->distributions->sp[c] = 0;" << endl;
    out << "    }" << endl;
    out << "}" << endl;
    out << "" << endl;
    out << "/**" << endl;
    out << " * printMinimalDistributions ()" << endl;
    out << " * Output all minimal distributions to the supplied stream." << endl;
    out << " */" << endl;
    out << "void printMinimalDistributions(std::ostream &out)" << endl;
    out << "{" << endl;
    out << "    out << \"<?xml version='1.0' encoding='UTF-8'?>\" << endl;" << endl;
    out << "    out << \"<sdf3 type='sdf' version='1.0'\" << endl;" << endl;
    out << "    out << \"xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'\" << endl;" << endl;
    out << "    out << \"xsi:noNamespaceSchemaLocation='http://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd'>\" << endl;" << endl;
    out << "    out << \"    <storageThroughputTradeOffs>\" << endl;" << endl;
    out << "" << endl;
    out << "    for (DistributionSet *p = minStorageDistributions; p != NULL;" << endl;
    out << "            p = p->next)" << endl;
    out << "    {" << endl;
    out << "        out << \"        <distributionsSet thr='\" << p->thr;" << endl;
    out << "        out << \"' sz='\" << p->sz << \"'>\" << endl;" << endl;
    out << "        for (Distribution *d = p->distributions; d != NULL; d = d->next)" << endl;
    out << "        {" << endl;
    out << "            out << \"            <distribution>\" << endl;" << endl;
    out << "            for (uint c = 0; c < NR_CHANNELS; c++)" << endl;
    out << "            {" << endl;
    out << "                out << \"            <ch name='\" << c << \"' sz='\" << d->sp[c];" << endl;
    out << "                out << \"'/>\" << endl;" << endl;
    out << "            }" << endl;
    out << "            out << \"            </distribution>\" << endl;" << endl;
    out << "        }" << endl;
    out << "        out << \"        </distributionsSet>\" << endl;" << endl;
    out << "    }" << endl;
    out << "    out << \"    </storageThroughputTradeOffs>\" << endl;" << endl;
    out << "    out << \"</sdf3>\" << endl;" << endl;
    out << "}" << endl;
    out << "" << endl;
}

static
void printMain(ostream &out)
{
    out << "int main(int argc, char **argv)" << endl;
    out << "{" << endl;
    out << "    CTimer timer;" << endl;
    out << "    " << endl;
    out << "    // Start timer" << endl;
    out << "    startTimer(&timer);" << endl;
    out << "    " << endl;
    out << "    // Search the space" << endl;
    out << "    findMinimalStorageDistributions(TDTIME_MAX);" << endl;
    out << "" << endl;
    out << "    // Stop timer" << endl;
    out << "    stopTimer(&timer);" << endl;
    out << "    " << endl;
    out << "    // Output the minimal storage distributions" << endl;
    out << "    printMinimalDistributions(cout);" << endl;
    out << " " << endl;
    out << "    // Output performance data" << endl;
    out << "    cerr << \"Analysis time:      \";" << endl;
    out << "    printTimer(cerr, &timer);" << endl;
    out << "    cerr << endl;" << endl;
    out << "    cerr << \"#Distr. checked:    \" << nrDistributionsChecked << endl;" << endl;
    out << "    cerr << \"Max stored states:  \" << maxNrStatesStored << endl;" << endl;
    out << "    cerr << \"Max visited states: \" << maxNrStatesVisisted << endl;" << endl;
    out << "    cerr << \"#actors:            \" << NR_ACTORS << endl;" << endl;
    out << "    cerr << \"#channels:          \" << NR_CHANNELS << endl;" << endl;
    out << "    cerr << \"#actors (HSDFG):    \" << NR_ACTORS_HSDFG << endl;" << endl;
    out << "    " << endl;
    out << "    return 0;" << endl;
    out << "}" << endl;
}

extern
void outputCSDFGasBuffyModel(TimedCSDFgraph *g, ostream &out)
{
    printDefinitions(g, out);
    printMiscFunctions(out);
    printState(out);
    printDependencies(out);
    printCSDFGexecution(out);
    printDistribution(g, out);
    printMain(out);
}

