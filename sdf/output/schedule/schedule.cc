/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   schedule.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   Generate self-timed scheduling model
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *
 * $Id: schedule.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "schedule.h"

static
SDFtime getMaximumExecTime(TimedSDFgraph *g)
{
    SDFtime max = 0;
    
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;

        max = a->getExecutionTime() > max ? a->getExecutionTime() : max;
    }
    
    return max;
}

static
void printDefinitions(TimedSDFgraph *g, ostream &out)
{
    out << "#define SDF_NUM_ACTORS          " << g->nrActors() << endl;
    out << "#define SDF_NUM_CHANNELS        " << g->nrChannels() << endl;
    out << "" << endl;
    out << "#define ACTOR_MAX_EXEC_TIME     " << getMaximumExecTime(g) << endl;
    out << "" << endl;
    out << "typedef unsigned int uint;" << endl;
    out << "typedef unsigned long long TTime;" << endl;
    out << "typedef unsigned long long TSize;" << endl;
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

void printSdfHeader(TimedSDFgraph *g, ostream &out)
{
    out << "/******************************************************************************" << endl;
	out << " * SDF" << endl;
	out << " *****************************************************************************/" << endl;
	out << "" << endl;
	out << "TSize ch[SDF_NUM_CHANNELS];" << endl;
	out << "TTime actClk[SDF_NUM_ACTORS][ACTOR_MAX_EXEC_TIME+1];" << endl;
    out << "bool actReady[SDF_NUM_ACTORS];" << endl;
	out << "" << endl;
	out << "#define CH(c)               (ch[c])" << endl;
	out << "#define CH_TOKENS(c,n)      (CH(c) >= n)" << endl;
	out << "#define CONSUME(c,n)        CH(c) = CH(c) - n;" << endl;
	out << "#define PRODUCE(c,n)        CH(c) = CH(c) + n;" << endl;
	out << "#define FIRE_ACT(a,t)       actClk[a][t]++; firing = true; actReady[a] = false;" << endl;
	out << "#define ACT_READY(a)        (concurrent || actReady[a])" << endl;
	out << "#define ACT_FIRED(a)        if (actClk[a][0] != 0) actReady[a] = true;" << endl;
	out << "" << endl;
}

static
void printSdfGraph(TimedSDFgraph *g, ostream &out)
{
    out << "void execSDFgraph(const TTime maxGlbClk, const bool concurrent)" << endl;
    out << "{" << endl;
    out << "    bool firing;" << endl;
    out << "    TTime glbClk = 1;" << endl;
    out << "" << endl;
    out << "    // No actors firing" << endl;
    out << "    for (uint a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "        for (uint t = 0; t <= ACTOR_MAX_EXEC_TIME; t++)" << endl;
    out << "            actClk[a][t] = 0;" << endl;
    out << "" << endl;
    out << "    // All actors ready to fire" << endl;
    out << "    for (uint a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "        actReady[a] = true;" << endl;
    out << "" << endl;
    out << "    // Initial tokens" << endl;

    for (SDFchannelsIter iter = g->channelsBegin(); iter != g->channelsEnd();
            iter++)
    {
        SDFchannel *ch = *iter;

        out << "    ch[" << ch->getId() << "] = " << ch->getInitialTokens() << ";" << endl;
    }

    out << "" << endl;
    out << "    while (glbClk < maxGlbClk)" << endl;
    out << "    {" << endl;
    out << "        cout << endl;" << endl;
    out << "        cout << glbClk << \": \";" << endl;
    out << "" << endl;        
    out << "        // Fire all actors till no actor fires anymore" << endl;
    out << "        do" << endl;
    out << "        {" << endl;
    out << "            // No actor has fired" << endl;
    out << "            firing = false;" << endl;
    out << "" << endl;
    out << "            // Check each actor once" << endl;

    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;

        out << "            if (ACT_READY(" << a->getId() << ")";
        
        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::In)
            {
                out << " && CH_TOKENS(" << ch->getId() << "," << p->getRate();
                out << ")";
            }    
        }
        out << ")" << endl;
        out << "            {" << endl;

        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::In)
            {
                out << "                CONSUME(" << ch->getId() << "," << p->getRate();
                out << ");" << endl;
            }    
        }

        out << "                FIRE_ACT(" << a->getId() << ",";
        out << a->getExecutionTime() << ");" << endl;
        out << "                cout << \"" << a->getName() << "\";" << endl;
        out << "            }" << endl;
    }

    out << "" << endl;
    out << "        }" << endl;
    out << "        while (firing == true);  " << endl; 
    out << "" << endl;
    out << "        // Move to next time step" << endl;
    out << "        glbClk++;" << endl;
    out << "        for (uint a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "        {" << endl;
    out << "            for (uint t = 0; t < ACTOR_MAX_EXEC_TIME; t++)" << endl;
    out << "                actClk[a][t] = actClk[a][t+1];" << endl;
    out << "            actClk[a][ACTOR_MAX_EXEC_TIME] = 0;" << endl;
    out << "        }" << endl;
    out << "" << endl;
    out << "        // Produce tokens for all completed actor firings" << endl;

    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();
            iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)*iter;

        for (SDFportsIter pIter = a->portsBegin(); pIter != a->portsEnd();
                pIter++)
        {
            SDFport *p = *pIter;
            SDFchannel *ch = p->getChannel();

            if (p->getType() == SDFport::Out)
            {
                out << "        PRODUCE(" << ch->getId() << ", actClk[" << a->getId() << "][0]*" << p->getRate();
                out << ");" << endl;
            }    
        }
    }

    out << "" << endl;
    out << "        // Actors which firing has finished are ready to fire again" << endl;
    out << "        for (uint a = 0; a < SDF_NUM_ACTORS; a++)" << endl;
    out << "            ACT_FIRED(a);" << endl;
    out << "" << endl;
    out << "    }" << endl;
    out << "" << endl;
    out << "    cerr << endl;" << endl;
    out << "}" << endl;
    out << "" << endl;
}

void printSdf(TimedSDFgraph *g, ostream &out)
{
    printSdfHeader(g, out);
    printSdfGraph(g, out);
}

static
void printMain(ostream &out, bool paretoSpace)
{
	out << "int main(int argc, char **argv)" << endl;
	out << "{" << endl;
    out << "    TTime maxGlbClk;" << endl;
    out << "    bool conc = false;" << endl;
    out << "" << endl;
    out << "    // Get maximum global clock" << endl;
    out << "    if (argc < 2)" << endl;
    out << "        exit(\"No maximum global clock specified.\", 1);" << endl;
    out << "" << endl;
    out << "    maxGlbClk = strtoull(argv[1], NULL, 10);" << endl;
    out << "" << endl;
    out << "    // Concurrent execution?" << endl;
    out << "    if (argc == 3 && strcmp(\"--conc\", argv[2]) == 0)" << endl;
    out << "        conc = true;" << endl;
    out << "" << endl;
	out << "    // Execute the SDF graph" << endl;
    out << "    execSDFgraph(maxGlbClk, conc);" << endl;
    out << "" << endl;
	out << "    return 0;" << endl;
	out << "}" << endl;
}

/**
 * outputSDFasSelfTimedScheduleModel ()
 * Output the SDF graph as a self-timed scheduling model.
 */
extern
void outputSDFasSelfTimedScheduleModel(TimedSDFgraph *g, ostream &out)
{
    printDefinitions(g, out);
    printMiscFunctions(out);
    printSdf(g, out);
    printMain(out, true);
}

