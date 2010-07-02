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
 *  Function        :   (C)SDFG analysis algorithms
 *
 *  History         :
 *      23-04-07    :   Initial version.
 *
 * $Id: sdf3analysis.cc,v 1.2 2008/03/22 14:24:21 sander Exp $
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
#include "sdf/sdf.h"
#include "csdf/csdf.h"

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
    out << "   --graph  <file>     input CSDF graph" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
    out << "   --algo <algorithm>  analyze the graph with requested algorithm:";
    out << endl;
    out << "       consistency" << endl;              
    out << "       repetition_vector" << endl;              
    out << "       repetition_vector_sum" << endl;          
    out << "       throughput" << endl;              
    out << "       buffersize" << endl;
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
}

/**
 * analyzeCSDFG ()
 * The function analyzes the CSDF graph.
 */
void analyzeCSDFG(TimedCSDFgraph *g, CPairs &analyze, ostream &out)
{
    CTimer timer;
 
    // Measure execution time
    startTimer(&timer);

    // Run analysis    
    if (analyze.front().key == "consistency")
    {
        if (!g->isConsistent())
            out << "Graph is not consistent." << endl;
        else
            out << "Graph is consistent." << endl;
    }
    else if (analyze.front().key == "repetition_vector")
    {
        RepetitionVector repetitionVector;
        repetitionVector = g->getRepetitionVector();

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
        repetitionVector = g->getRepetitionVector();

        for (uint i = 0; i < repetitionVector.size(); i++)
            sum += repetitionVector[i];
        
        out << "Repetition vector sum: " << sum << endl;
    }
    else if (analyze.front().key == "throughput")
    {   
        CSDFstateSpaceThroughputAnalysis thrAlgo;
        double thr;

        thr = thrAlgo.analyze(g);
        out << "thr(" << g->getName() << ") = " << thr << endl;
    }
    else if (analyze.front().key == "buffersize")
    {
        StorageDistributionSet *minStorageDistributions;
        CSDFstateSpaceBufferAnalysis bufferAlgo;
        
        // Explore trade-off space
        minStorageDistributions = bufferAlgo.analyze(g);

        out << "<?xml version='1.0' encoding='UTF-8'?>" << endl;
        out << "<sdf3 type='sdf' version='1.0'" << endl;
        out << "     xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'";
        out << endl;
        out << "     xsi:noNamespaceSchemaLocation='http://www.es.ele.tue.nl/sdf3/xsd/sdf3-csdf.xsd'>" << endl;
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
    else
    {
        throw CException("Unknown analysis algorithm.");
    }
    
    // Measure execution time
    stopTimer(&timer);
    out << "analysis time: ";
    printTimer(out, &timer);
    out << endl;
}

/**
 * analyzeCSDFG ()
 * The function analyzes the CSDF graph.
 */
void analyzeCSDFG(ostream &out)
{
    CNode *csdfNode, *csdfPropertiesNode;
    TimedCSDFgraph *csdfGraph;

    // Find csdf graph in XML structure
    csdfNode = CGetChildNode(settings.xmlAppGraph, "csdf");
    if (csdfNode == NULL)
        throw CException("Invalid xml file - missing 'csdf' node");
    csdfPropertiesNode = CGetChildNode(settings.xmlAppGraph, "csdfProperties");
    
    // Construction CSDF graph model
    csdfGraph = constructTimedCSDFgraph(csdfNode, csdfPropertiesNode);
    
    // The actual analysis...
    analyzeCSDFG(csdfGraph, settings.arguments, out);
    
    // Cleanup
    delete csdfGraph;
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
        analyzeCSDFG(out);
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
