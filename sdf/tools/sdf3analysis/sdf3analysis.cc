/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3analysis.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 23, 2007
 *
 *  Function        :   SDFG analysis algorithms
 *
 *  History         :
 *      23-04-07    :   Initial version.
 *
 * $Id: sdf3analysis.cc,v 1.9 2008/09/25 10:49:58 sander Exp $
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

#include "sdf3analysis.h"
#include "../../sdf.h"

typedef struct _CPair
{
    CString key;
    CString value;
} CPair;

typedef list<CPair>         CPairs;
typedef CPairs::iterator    CPairsIter;

/**
 * Settings
 * Struct to store program settings.
 */
typedef struct _Settings
{
    // Input file with graph
    CString graphFile;
    
    // Output file
    CString outputFile;
    
    // Switch argument(s) given to analysis algorithm
    CPairs arguments;
    
    // Application graph
    CNode *xmlAppGraph;
    
    // Architecture graph
    CNode *xmlArchGraph;
    
    // Mapping
    CNode *xmlMapping;    
} Settings;

/**
 * settings
 * Program settings.
 */
Settings settings;

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(ostream &out)
{
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
    out << endl;
    out << "Usage: " << TOOL << " --graph <file> --algo <algorithm>";
    out << " [--output <file>]";
    out << endl;
    out << "   --graph  <file>     input SDF graph" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
    out << "   --algo <algorithm>  analyze the graph with requested algorithm:";
    out << endl;
    out << "       consistency" << endl;
    out << "       deadlock" << endl;              
    out << "       simple_cycles" << endl;              
    out << "       acyclic_graph" << endl;              
    out << "       connected_graph" << endl;              
    out << "       repetition_vector" << endl;              
    out << "       repetition_vector_sum" << endl;          
    out << "       strongly_connected_components" << endl;  
    out << "       mcm[(cycle,dasdan,karp,howard,yto,yto-mcr)]" << endl;
    out << "       is_hsdf" << endl;              
    out << "       statistics" << endl;              
    out << "       throughput" << endl;              
    out << "       buffersize" << endl;
    out << "       buffersize_ning_gao" << endl;           
    out << "       buffersize_capacity_constrained" << endl;           
    out << "       latency(method,srcActor,dstActor)" << endl;
    out << "       binding_aware_throughput([NSoC,MPFlow])" << endl;
    out << "       static_periodic_schedule" << endl;
    out << "       static_periodic_schedule_chao" << endl;       
}

/**
 * parseSwitchArgument ()
 * The function parses the string 'arguments' into a sequence of (arg, value)
 * pairs. The syntax as as follows:
 *
 * pair := key(value)
 * arg := pair,pair,...
 *
 * Note: value may be a pair itself, but this is not expanded into a set of
 * pairs (i.e. nested pairs are not supported).
 */
CPairs parseSwitchArgument(CString arguments)
{
    CPairs pairs;
    CPair p;
    
    while (arguments.size() != 0)
    {
        char c;
        p.key = "";
        p.value = "";
        
        // Get key from argument string
        do
        {
            c = arguments[0];
            arguments = arguments.substr(1);
            if (c == ',' || c== '(')
                break;
            p.key += c;
        } while (arguments.size() != 0);

        // Is next part of argument a value?
        if (c == '(')
        {
            CString::size_type ePos = 0;
            int level = 1;
            
            // Find the matching closing brace
            while (level != 0 && arguments.size() != 0)
            {
                if (arguments[ePos] == ')')
                    level--;
                else if (arguments[ePos] == '(')
                    level++;
                
                // Next
                ePos++;
            }
            
            // Closing brace found?
            if (level != 0)
                throw CException("Missing closing brace in value of argument.");
              
            // Get value  
            p.value = arguments.substr(0, ePos-1);
            
            // More arguments left?
            if (arguments.size() > ePos)
                arguments = arguments.substr(ePos+1); 
            else
                arguments = "";
        }

        // Add pair to list of pairs
        pairs.push_back(p);
    }

    return pairs;
}

/**
 * parseCommandLine ()
 * The function parses the command line arguments and add info to the
 * supplied settings structure.
 */
void parseCommandLine(int argc, char ** argv)
{
    int arg = 1;
    
    if (argc == 1)
    {
        helpMessage(cerr);
        throw CException("");
    }
    
    do
    {
        // Configuration file
        if (argv[arg] == CString("--graph") && arg+1<argc)
        {
            arg++;
            settings.graphFile = argv[arg];
        }
        else if (argv[arg] == CString("--output") && arg+1<argc)
        {
            arg++;
            settings.outputFile = argv[arg];
        }
        else if (argv[arg] == CString("--algo") && arg+1<argc)
        {
            arg++;
            settings.arguments = parseSwitchArgument(argv[arg]);
        }
        else
        {
            helpMessage(cerr);
            throw CException("");
        }
        
        // Next argument
        arg++;
    } while (arg < argc);
}

/**
 * loadApplicationGraphFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the SDFG.
 */
CNode *loadApplicationGraphFromFile(CString &file, CString module)
{
    CNode *appGraphNode, *sdf3Node;
    CDoc *appGraphDoc;
    
    // Open file
    appGraphDoc = CParseFile(file);
    if (appGraphDoc == NULL)
        throw CException("Failed loading application from '" + file + "'.");

    // Locate the sdf3 root element and check module type
    sdf3Node = CGetRootNode(appGraphDoc);
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + file + "' is not "
                         "of type '" + module + "'.");
    }
    
    // Get application graph node
    appGraphNode = CGetChildNode(sdf3Node, "applicationGraph");
    if (appGraphNode == NULL)
        throw CException("No application graph in '" + file + "'.");
    
    return appGraphNode;
}

/**
 * loadArchitectureGraphFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the architecture.
 */
CNode *loadArchitectureGraphFromFile(CString &file, CString module)
{
    CNode *archGraphNode, *sdf3Node;
    CDoc *archGraphDoc;
    
    // Open file
    archGraphDoc = CParseFile(file);
    if (archGraphDoc == NULL)
        throw CException("Failed loading application from '" + file + "'.");

    // Locate the sdf3 root element and check module type
    sdf3Node = CGetRootNode(archGraphDoc);
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + file + "' is not "
                         "of type '" + module + "'.");
    }
    
    // Get architecture graph node
    archGraphNode = CGetChildNode(sdf3Node, "architectureGraph");
    
    return archGraphNode;
}

/**
 * loadMappingFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the mapping.
 */
CNode *loadMappingFromFile(CString &file, CString module)
{
    CNode *mappingNode, *sdf3Node;
    CDoc *mappingDoc;
    
    // Open file
    mappingDoc = CParseFile(file);
    if (mappingDoc == NULL)
        throw CException("Failed loading application from '" + file + "'.");

    // Locate the sdf3 root element and check module type
    sdf3Node = CGetRootNode(mappingDoc);
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + file + "' is not "
                         "of type '" + module + "'.");
    }
    
    // Get mapping node
    mappingNode = CGetChildNode(sdf3Node, "mapping");
    
    return mappingNode;
}

/**
 * initSettings ()
 * The function initializes the program settings.
 */
void initSettings(int argc, char **argv)
{
    // Parse the command line
    parseCommandLine(argc, argv);

    // Check required settings
    if (settings.graphFile.empty() || settings.arguments.size() == 0)
    {
        helpMessage(cerr);
        throw CException("");
    }

    // Load application graph
    settings.xmlAppGraph = loadApplicationGraphFromFile(settings.graphFile,
                                                                        MODULE);

    // Load architecture graph
    settings.xmlArchGraph = loadArchitectureGraphFromFile(settings.graphFile,
                                                                        MODULE);

    // Load mapping
    settings.xmlMapping = loadMappingFromFile(settings.graphFile, MODULE);
}

/**
 * analyzeSDFG ()
 * The function analyzes the SDF graph.
 */
void analyzeSDFG(TimedSDFgraph *g, CPairs &analyze, ostream &out)
{
    CTimer timer;
    
    if (analyze.front().key == "consistency")
    {
        if (!isSDFgraphConsistent(g))
            out << "Graph is not consistent." << endl;
        else
            out << "Graph is consistent." << endl;
    }
    else if (analyze.front().key == "deadlock")
    {
        SDFstateSpaceDeadlockAnalysis deadlockAlgo;

        if (!deadlockAlgo.isDeadlockFree(g))
            out << "Graph deadlocks." << endl;
        else
            out << "Graph is deadlock free." << endl;
    }
    else if (analyze.front().key == "repetition_vector")
    {
        RepetitionVector repetitionVector;
        repetitionVector = computeRepetitionVector(g);

        out << "Repetition vector:" << endl;
        for (uint i = 0; i < repetitionVector.size(); i++)
        {
            out << " [" << g->getActor(i)->getName() << "] = ";
            out << repetitionVector[i] << endl;
        }
    }
    else if (analyze.front().key == "repetition_vector_sum")
    {
        uint sum = 0;
        
        RepetitionVector repetitionVector;
        repetitionVector = computeRepetitionVector(g);

        for (uint i = 0; i < repetitionVector.size(); i++)
            sum += repetitionVector[i];
        
        out << "Repetition vector sum: " << sum << endl;
    }
    else if (analyze.front().key == "strongly_connected_components")
    {
        SDFgraphComponents components;
        components = stronglyConnectedComponents(g);

        out << "Strongly connected components:" << endl;
        for (SDFgraphComponentsIter iter = components.begin(); 
                iter != components.end(); iter++)
        {
            bool first = true;
            SDFgraphComponent &comp = *iter;

            out << "Component: ";
            for (SDFactorsIter iterA = comp.begin(); 
                    iterA != comp.end(); iterA++)
            {
                SDFactor *a = *iterA;

                if (!first)
                    out << ", ";
                else
                    first = false;

                out << a->getName();
            }
            out << endl;
        }
    }
    else if (analyze.front().key == "simple_cycles")
    {
        SDFgraphCycles cycles;
        cycles = findSimpleCycles(g);

        out << "Simple cycles:" << endl;
        for (SDFgraphCyclesIter iter = cycles.begin(); 
                iter != cycles.end(); iter++)
        {
            bool first = true;
            SDFgraphCycle cycle = *iter;

            out << "Cycle: ";
            for (SDFactorsIter iterA = cycle.begin();
                    iterA != cycle.end(); iterA++)
            {
                SDFactor *a = *iterA;

                if (!first)
                    out << ", ";
                else
                    first = false;

                out << a->getName();
            }
            out << endl;
        }
    }
    else if (analyze.front().key == "mcm")
    {   
        CFraction mcm;
        TimedSDFgraph *hsdfGraph;

        // Measure execution time
        startTimer(&timer);

        hsdfGraph = (TimedSDFgraph*)transformSDFtoHSDF(g);

        if (analyze.front().value.empty() || analyze.front().value == "cycle")
        {
            mcm = maximumCycleMeanCycles(hsdfGraph);
        }
        else if (analyze.front().value == "dasdan")
        {
            mcm = maximumCycleMeanDasdanGupta(hsdfGraph);
        }
        else if (analyze.front().value == "karp")
        {
            mcm = maximumCycleMeanKarp(hsdfGraph);
        }
        else if (analyze.front().value == "howard")
        {
            mcm = maximumCycleMeanHoward(hsdfGraph);
        }
        else if (analyze.front().value == "yto")
        {
            mcm = maximumCycleYoungTarjanOrlin(hsdfGraph, true);
        }
        else if (analyze.front().value == "yto-mcr")
        {
            mcm = maximumCycleYoungTarjanOrlin(hsdfGraph, false);
        }
        else
        {
            throw CException("Unknown MCM algorithm.");
        }

        // Measure execution time
        stopTimer(&timer);
        
        out << "MCM(" << g->getName() << ") = " << mcm << endl;

        out << "analysis time: ";
        printTimer(out, &timer);
        out << endl;
        
        delete hsdfGraph;
    }
    else if (analyze.front().key == "throughput")
    {   
        SDFstateSpaceThroughputAnalysis thrAnalysisAlgo;
        double thr;

        // Measure execution time
        startTimer(&timer);
       
        thr = thrAnalysisAlgo.analyze(g);

        // Measure execution time
        stopTimer(&timer);
        
        out << "thr(" << g->getName() << ") = " << thr << endl;

        out << "analysis time: ";
        printTimer(out, &timer);
        out << endl;
    }
    else if (analyze.front().key == "acyclic_graph")
    {
        if (isAcyclic(g))
            out << "Graph is acyclic." << endl;
        else
            out << "Graph is cyclic." << endl;
    }
    else if (analyze.front().key == "connected_graph")
    {
        if (isConnected(g))
            out << "Graph is a connected graph." << endl;
        else
            out << "Graph is not a connected graph." << endl;
    }
    else if (analyze.front().key == "is_hsdf")
    {
        if (isHSDFgraph(g))
            out << "Graph is a HSDF graph." << endl;
        else
            out << "Graph is not a HSDF graph." << endl;
    }
    else if (analyze.front().key == "statistics")
    {
        long actCntHsdf = 0, chCntHsdf = 0;
        
        RepetitionVector repVec = computeRepetitionVector(g);
        
        for (uint i = 0; i < g->nrActors(); i++)
            actCntHsdf += repVec[i];
        
        for (SDFactorsIter iter = g->actorsBegin(); 
            iter != g->actorsEnd(); iter++)
        {
            SDFactor *a = *iter;
            
            for (SDFportsIter iterP = a->portsBegin(); 
                    iterP != a->portsEnd(); iterP++)
            {
                SDFport *p = *iterP;
                
                if (p->getType() == SDFport::In)
                    chCntHsdf += p->getRate()*repVec[a->getId()];
            }
        }
        
        out << "#actors:          " << g->nrActors() << endl;
        out << "#channels:        " << g->nrChannels() << endl;
        out << "#actors (HSDF):   " << actCntHsdf << endl;
        out << "#channels (HSDF): " << chCntHsdf << endl;
    }
    else if (analyze.front().key == "buffersize")
    {
        StorageDistributionSet *minStorageDistributions;
        SDFstateSpaceBufferAnalysis bufferAnalysisAlgo;
        
        minStorageDistributions = bufferAnalysisAlgo.analyze(g);

        out << "<?xml version='1.0' encoding='UTF-8'?>" << endl;
        out << "<sdf3 type='sdf' version='1.0'" << endl;
        out << "     xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'" << endl;
        out << "      xsi:noNamespaceSchemaLocation='http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd'>" << endl;
        out << "    <storageThroughputTradeOffs>" << endl;

        for (StorageDistributionSet *p = minStorageDistributions; p != NULL;
                p = p->next)
        {
            out << "        <distributionsSet thr='";
            out << p->thr;
            out << "' sz='" << p->sz << "'>" << endl;
            for (StorageDistribution *d = p->distributions;
                    d != NULL; d = d->next)
            {
                out << "            <distribution>" << endl;
                for (uint c = 0; c < g->nrChannels(); c++)
                {
                    out << "                <ch name='";
                    out << g->getChannel(c)->getName();
                    out << "' sz='" << d->sp[c];
                    out << "'/>" << endl;
                }
                out << "            </distribution>" << endl;
            }
            out << "        </distributionsSet>" << endl;
        }
        out << "    </storageThroughputTradeOffs>" << endl;
        out << "</sdf3>" << endl;
    }
    else if (analyze.front().key == "buffersize_ning_gao")
    {
        SDFstateSpaceBufferAnalysisNingGao bufferAnalysisAlgo;
        TimedSDFgraph *gConstrained;
        vector<SDFtime> startTime;
        StorageDistribution *d;
        
        // Create a capacity constrained model        
        gConstrained = createCapacityConstrainedModel(g);
        
        // Analayze storage space requirements of the capacity constrained graph
        d = bufferAnalysisAlgo.analyze(gConstrained, startTime);

        // Output the storage distribution
        int sz = 0;
        for (uint a = 0; a < g->nrActors(); a++)
        {
            sz += d->sp[a];
            out << "b(" << g->getActor(a)->getName() << ") = " << d->sp[a] << endl;
        }
        for (uint a = 0; a < g->nrActors(); a++)
        {
            out << "s(" << g->getActor(a)->getName() << ") = " << startTime[a] << endl;
        }
        out << "size =   " << sz << endl;
        out << "period = " << 1.0 / d->thr << endl;
        
        // Create a static-periodic schedule
        SDFstateSpaceThroughputAnalysisNingGao thrAnalysis;
        TDtime thr = thrAnalysis.analyze(g, d, SDFtime(1.0/d->thr), startTime);
        if (thr != d->thr)
        {
            throw CException("Static-periodic schedule is invalid");
        }
        
        // Cleanup
        delete gConstrained;
    }
    else if (analyze.front().key == "buffersize_ning_gao_hijdra")
    {
        SDFstateSpaceBufferAnalysisNingGao bufferAnalysisAlgo;
        TimedSDFgraph *gConstrained;
        vector<SDFtime> startTime;
        StorageDistribution *d;

        // Measure execution time
        startTimer(&timer);
        
        // Create a capacity constrained model        
        gConstrained = createCapacityConstrainedModel(g);

        // Analayze storage space requirements of the capacity constrained graph
        d = bufferAnalysisAlgo.analyze(gConstrained, startTime);

        // Measure execution time
        stopTimer(&timer);

        // Output the graph in Hijdra format
        out << "actors" << endl;            
        out << endl;
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)(*iter);
            
            out << "name=\"" << a->getName() << "\" exec=";
            out << a->getExecutionTime() << ";" << endl;
        }
        out << endl;

        out << "arcs" << endl;            
        out << endl;
        for (SDFchannelsIter iter = g->channelsBegin();
                iter != g->channelsEnd(); iter++)
        {
            TimedSDFchannel *c = (TimedSDFchannel*)(*iter);
            
            out << "src=\"" << c->getSrcActor()->getName() << "\" ";
            out << "dst=\"" << c->getDstActor()->getName() << "\" ";
            out << "delay=" << c->getInitialTokens() << " ";
            out << "buffersize=" << d->sp[c->getSrcActor()->getId()];
            
            // Self-edge?
            if (c->getSrcActor()->getId() == c->getDstActor()->getId())
            {
                out << " tokensize=0;" << endl;
            }
            else
            {
                out << ";" << endl;
            }
        }
        out << endl;

        out << "constraints" << endl;
        out << endl;
        out << "mud=" << 1.0 / d->thr << endl;
        out << "(*" << endl;
        outputSDFasXML(g, out);
        out << "analysis time: ";
        printTimer(out, &timer);
        out << endl;
        out << "*)" << endl;
        out << "end" << endl;
    }
    else if (analyze.front().key == "buffersize_capacity_constrained")
    {
        StorageDistributionSet *minStorageDistributions;
        TimedSDFgraph *gNoConcurrency, *gConstrained;
        CFraction mcm;
        double maxThr;
        
        // Exclude auto-concurrency
        gNoConcurrency = (TimedSDFgraph*)modelAutoConcurrencyInSDFgraph(g, 1);

        // Compute maximal throughput and MCM
        mcm = maximumCycleMeanCycles(gNoConcurrency);
        mcm = mcm.lowestTerm();
        maxThr = 1.0 / mcm.value();

        // MCM must be an integer value
        if (mcm.denominator() != 1)
            throw CException("MCM must have an integer value.");

        // Create capacity constrained model
        gConstrained = modelCapacityConstrainedBuffer(gNoConcurrency,
                                                            mcm.numerator());
        
        // Analayze storage space requirements of the capacity constrained graph
        minStorageDistributions =
                stateSpaceBufferAnalysisCapacityConstrainedModel(gConstrained,
                                                                    maxThr);

        // Output all minimal storage distributions
        out << "<?xml version='1.0' encoding='UTF-8'?>" << endl;
        out << "<sdf3 type='sdf' version='1.0'" << endl;
        out << "     xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'";
        out << endl;
        out << "     xsi:noNamespaceSchemaLocation='http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd'>" << endl;
        out << "    <storageThroughputTradeOffs>" << endl;

        for (StorageDistributionSet *p = minStorageDistributions; p != NULL;
                p = p->next)
        {
            out << "        <distributionsSet thr='";
            out << p->thr;
            out << "' sz='" << p->sz << "'>" << endl;
            for (StorageDistribution *d = p->distributions;
                    d != NULL; d = d->next)
            {
                out << "            <distribution>" << endl;
                for (uint c = 0; c < gConstrained->nrChannels(); c++)
                {
                    // Channel models storage space?
                    if (((TimedSDFchannel*)gConstrained->getChannel(c))
                            ->modelsStorageSpace())
                    {
                        TimedSDFchannel *ch;
                        ch = (TimedSDFchannel*)gConstrained->getChannel(c);
                        ch = ch->getStorageSpaceChannel();
                        
                        // Channel ch of which storage space is modeled with
                        // channel c appears in the original graph g?
                        if (g->getChannel(ch->getName()) != NULL)
                        {
                            // Channel is not a self-edge added to remove
                            // auto-concurrency
                            out << "                <ch name='";
                            out << ch->getName();
                            out << "' sz='" << d->sp[c];
                            out << "'/>" << endl;
                        }
                    }
                }
                out << "            </distribution>" << endl;
            }
            out << "        </distributionsSet>" << endl;
        }
        out << "    </storageThroughputTradeOffs>" << endl;
        out << "</sdf3>" << endl;
        
        // Cleanup
        delete gNoConcurrency;
        delete gConstrained;
    }
    else if (analyze.front().key == "latency")
    {
        CString srcActorName, dstActorName, method;
        SDFactor *srcActor, *dstActor;
        CStrings options;
        double latency, thr = -1.0;

        // Extract options supplied with latency switch
        stringtok(options, analyze.front().value, ",");
        if (options.size() != 3)
            throw CException("Incorrect number of options given.");
        method = options.front(); options.pop_front();
        srcActorName = options.front(); options.pop_front();
        dstActorName = options.front(); options.pop_front();
        
        // Find pointer to src and dst actor in graph
        srcActor = g->getActor(srcActorName);
        if (srcActor == NULL)
            throw CException("No source actor found.");
        dstActor = g->getActor(dstActorName);
        if (dstActor == NULL)
            throw CException("No destination actor found.");
        
        // Measure execution time
        startTimer(&timer);
        
        // Compute latency
        if (method == "st")
        {
            SDFstateSpaceSelfTimedLatencyAnalysis selftimedLatencyAnalysisAlgo;
            
            selftimedLatencyAnalysisAlgo.analyze(g, srcActor, dstActor, 
                                                    latency, thr);
        }
        else if (method == "sp")
        {
            SDFstateSpaceMinimalLatencyAnalysis stateSpaceMinLatencyAnalysis;
            
            stateSpaceMinLatencyAnalysis.analyzeSingleProc(g, srcActor,
                                                            dstActor, latency);
        }
        else if (method == "min")
        {
            SDFstateSpaceMinimalLatencyAnalysis stateSpaceMinLatencyAnalysis;
            
            stateSpaceMinLatencyAnalysis.analyze(g, srcActor, dstActor, 
                                                            latency, thr);
        }
        else if (method == "ro")
        {

            latency = latencyAnalysisForRandomStaticOrderSingleProc(
                                                   g, srcActor, dstActor, 100);
        }
        else if (method == "min_st")
        {
            SDFstateSpaceSelfTimedMinimalLatencyAnalysis
                                            selftimedMinimalLatencyAnalysisAlgo;
            
            selftimedMinimalLatencyAnalysisAlgo.analyze(g, srcActor, dstActor,
                                                        latency, thr);
        }
        else
        {
            throw CException("Latency analysis method unknown.");
        }

        // Measure execution time
        stopTimer(&timer);
                
        out << "latency(" << g->getName() << ") = " << latency << endl;
        
        if (thr != -1)
            out << "throughput(" << g->getName() << ") = " << thr << endl;
        
        out << "analysis time: ";
        printTimer(out, &timer);
        out << endl;
    }
    else if (analyze.front().key == "binding_aware_throughput")
    {
        SDFflowType flowType;
        BindingAwareSDFG *bindingAwareSDFG;
        PlatformGraph *platformGraph;
        SDFstateSpaceBindingAwareThroughputAnalysis thrBindingAlgo;
        vector<double> tileUtilization;
        double thr;
        
        if (analyze.front().value ==  "NSoC")
        {
                flowType = SDFflowTypeNSoC;
        }
        else if (analyze.front().value ==  "MPFlow")
        {
                flowType = SDFflowTypeMPFlow;
        }
        else
        {
            throw CException("Flow type '" + analyze.front().value 
                                + "'is not supported.");
        }
        
        // Create a platform graph
        if (settings.xmlArchGraph == NULL)
            throw CException("No architectureGraph given.");
        platformGraph = constructPlatformGraph(settings.xmlArchGraph);
        
        // Set the mapping of the application onto the platform graph
        if (settings.xmlMapping == NULL)
            throw CException("No mapping given.");
        setMappingPlatformGraph(platformGraph, g, settings.xmlMapping);
        
        // Create a binding-aware SDFG
        bindingAwareSDFG = new BindingAwareSDFG(g, platformGraph, flowType);

        // Measure execution time
        startTimer(&timer);
        
        // Analayze the throughput
        thr = thrBindingAlgo.analyze(bindingAwareSDFG, tileUtilization);
        
        // Measure execution time
        stopTimer(&timer);

        out << "thr(" << g->getName() << ") = " << thr << endl;

        out << "analysis time: ";
        printTimer(out, &timer);
        out << endl;
        
        // Cleanup
        delete bindingAwareSDFG;
        delete platformGraph;
    }
    else if (analyze.front().key == "static_periodic_schedule")
    {
        SDFstateSpaceStaticPeriodicScheduler scheduler;
        
        scheduler.schedule(g);
    }
    else if (analyze.front().key == "static_periodic_schedule_chao")
    {
        SDFstateSpaceStaticPeriodicSchedulerChao scheduler;
        
        TimedSDFgraph *h = (TimedSDFgraph*)transformSDFtoHSDF(g);
        
        scheduler.schedule(h);
        
        delete h;
    }
    else
    {
        throw CException("Unknown analysis algorithm.");
    }
}

/**
 * analyzeSDFG ()
 * The function analyzes the SDF graph.
 */
void analyzeSDFG(ostream &out)
{
    TimedSDFgraph *sdfGraph;
    CNode *sdfNode, *sdfPropertiesNode;

    // Find sdf graph in XML structure
    sdfNode = CGetChildNode(settings.xmlAppGraph, "sdf");
    if (sdfNode == NULL)
        throw CException("Invalid xml file - missing 'sdf' node");
    sdfPropertiesNode = CGetChildNode(settings.xmlAppGraph, "sdfProperties");
    
    // Construction SDF graph model
    sdfGraph = new TimedSDFgraph();
    sdfGraph->construct(sdfNode, sdfPropertiesNode);
    
    // The actual analysis...
    analyzeSDFG(sdfGraph, settings.arguments, out);
    
    // Cleanup
    delete sdfGraph;
}

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    int exit_status = 0;
    ofstream out;

    try
    {
        // Initialize the program
        initSettings(argc, argv);

        // Set output stream
        if (!settings.outputFile.empty())   
            out.open(settings.outputFile.c_str());
        else
            ((ostream&)(out)).rdbuf(cout.rdbuf());
            
        // Perform requested actions
        analyzeSDFG(out);
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
