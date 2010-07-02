/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3transform.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 23, 2007
 *
 *  Function        :   SDFG transformation functionality
 *
 *  History         :
 *      23-04-07    :   Initial version.
 *
 * $Id: sdf3transform.cc,v 1.2 2007/11/02 09:34:46 sander Exp $
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

#include "sdf3transform.h"
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
    
    // Switch argument(s) given to algorithm
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
    out << "Usage: " << TOOL << " --graph <file> --transform <list>";
    out << " [--output <file>]" << endl;
    out << "   --graph  <file>     input SDF graph" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
    out << "   --transform <list>  comma separated list with graph";
    out << " transformations:" << endl;
    out << "       to_hsdf" << endl;
    out << "       unfold(<count>)" << endl;
    out << "       model_buffersize" << endl;
    out << "       model_autoconc(<max>)" << endl;
    out << "       select_buffersize" << endl;
    out << "       move_state" << endl;
    out << "       capacity_constrained(<mcm>)" << endl;
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
    
    while (arg < argc)
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
        else if (argv[arg] == CString("--transform") && arg+1<argc)
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
    }
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
 * transformGraph ()
 * The function transforms the SDF graph and the resulting (H)SDF graph
 * is outputted in XML format to the out stream.
 */
void transformGraph(TimedSDFgraph *g, CPairs &transforms, ostream &out)
{
    SDFcomponent component = SDFcomponent(g->getParent(), g->getId());
    TimedSDFgraph *h_in = g->clone(component), *h_out;
    
    for (CPairsIter iter = transforms.begin(); iter != transforms.end(); iter++)
    {
        CPair &transform = *iter;
        
        // Perform transformation
        if (transform.key == "to_hsdf")
        {
    	    h_out = (TimedSDFgraph*)transformSDFtoHSDF(h_in);
        }
        else if (transform.key == "unfold")
        {
            if (transform.value == "")
                throw CException("Missing unfolding factor.");
                
            h_out = (TimedSDFgraph*)unfoldHSDF(h_in, transform.value);
        }
        else if (transform.key == "model_buffersize")
        {
            h_out = modelBufferSizeInSDFgraph(h_in);
        }
        else if (transform.key == "model_autoconc")
        {
            if (transform.value == "")
                throw CException("Missing maximum degree of auto-concurrency.");
                
            h_out = (TimedSDFgraph*)modelAutoConcurrencyInSDFgraph(h_in,
                                                            transform.value);
        }
        else if (transform.key == "select_buffersize")
        {
            StorageDistributionSet *storageDistributions, *thrPoint;
            SDFstateSpaceBufferAnalysis bufferAnalysisAlgo;
            uint nrThrPoints = 0, nrDistPoints = 0, select;
            StorageDistribution *dist;
            
            // Load storage distributions from XML file
            storageDistributions = bufferAnalysisAlgo.analyze(h_in);
            
            // Print list of all throughput points
            for (StorageDistributionSet *d = storageDistributions;
                    d != NULL;d = d->next)
            {
                cout << "Throughput " << nrThrPoints << ": " << d->thr << endl;
                nrThrPoints++;
            }
            
            // Select throughput point
            if (nrThrPoints == 1)
            {
                select = 0;
            }
            else
            {
                select = nrThrPoints;
                while (select < 0 || select >= nrThrPoints)
                {
                    cout << "Select storage distribution (throughput): ";
                    std::cin >> select;
                }
                cout << endl;
            }
                        
            // Find throughput point
            nrThrPoints = 0;
            for (thrPoint = storageDistributions;
                    thrPoint != NULL; thrPoint = thrPoint->next)
            {
                if (nrThrPoints == select)
                    break;
                nrThrPoints++;
            }

            // Print list of all storage distributions with selected throughput
            for (StorageDistribution *d = thrPoint->distributions;
                    d != NULL; d = d->next)
            {
                cout << "Distribution " << nrDistPoints << ": " << endl;
                for (uint c = 0; c < h_in->nrChannels(); c++)
                    cout << "    " << d->sp[c] << endl;
                nrDistPoints++;
            }
            
            // Select distribution point
            if (nrDistPoints == 1)
            {
                select = 0;
            }
            else
            {
                select = nrDistPoints;
                while (select < 0 || select >= nrDistPoints)
                {
                    cout << "Select storage distribution: ";
                    std::cin >> select;
                }
                cout << endl;
            }
                        
            // Find distribution point
            nrDistPoints = 0;
            for (dist = thrPoint->distributions;
                    dist != NULL; dist = dist->next)
            {
                if (select == nrDistPoints)
                    break;
                nrDistPoints++;
            }
            
            // Create a copy of the graph and set buffer sizes of the channels
            SDFcomponent component(h_in->getParent(), 
                                        h_in->getId(), h_in->getName());
            h_out = h_in->clone(component);
            
            for (SDFchannelsIter iterCh = h_out->channelsBegin();
                    iterCh != h_out->channelsEnd(); iterCh++)
            {
                TimedSDFchannel *ch = (TimedSDFchannel*)(*iterCh);
                TimedSDFchannel::BufferSize buffersize;
                
                buffersize.sz = dist->sp[ch->getId()];
                buffersize.mem = 0;
                buffersize.src = 0;
                buffersize.dst = 0;
                ch->setBufferSize(buffersize);
            }
        }
        else if (transform.key == "move_state")
        {
            // Create a copy of the graph and set buffer sizes of the channels
            SDFcomponent component(h_in->getParent(), 
                                        h_in->getId(), h_in->getName());
            h_out = h_in->clone(component);
            
            for (SDFactorsIter iter = h_out->actorsBegin();
                    iter != h_out->actorsEnd(); iter++)
            {
                TimedSDFactor *a = (TimedSDFactor*)(*iter);
                TimedSDFchannel *c;
                TimedSDFchannel::BufferSize buffersize;
                uint state = UINT_MAX;
                
                buffersize.sz  = 1;
                buffersize.mem = 1;
                buffersize.src = 1;
                buffersize.dst = 1;
                
                for (TimedSDFactor::ProcessorsIter iterP = a->processorsBegin();
                        iterP != a->processorsEnd(); iterP++)
                {
                    TimedSDFactor::Processor *p = *iterP;
                    
                    if (p->stateSize < state)
                        state = p->stateSize;
                }
                
                c = h_out->createChannel(a, 1, a, 1, 1);
                c->setTokenSize(state);
                c->setInitialTokens(1);
                c->setBufferSize(buffersize);
                c->setMinBandwidth(0);
                c->setMinLatency(0);

                for (TimedSDFactor::ProcessorsIter iterP = a->processorsBegin();
                        iterP != a->processorsEnd(); iterP++)
                {
                    TimedSDFactor::Processor *p = *iterP;
                    
                    p->stateSize = p->stateSize - state;
                }
            }            
        }
        else if (transform.key == "capacity_constrained")
        {
            if (transform.value == "")
                throw CException("Missing MCM.");
                
            h_out = modelCapacityConstrainedBuffer(h_in, transform.value);
        }
        else
        {
            throw CException("Unknown transformation.");
        }
        
        // Output graph becomes next input
        delete h_in;
        h_in = h_out;
    }
    
    // Output resulting SDF graph
    outputSDFasXML(h_in, out);

    // Cleanup
    delete h_in;
}

/**
 * transformGraph ()
 * The function transforms the SDF graph.
 */
void transformGraph(ostream &out)
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
    
    // The actual transformation...
    transformGraph(sdfGraph, settings.arguments, out);
    
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
        transformGraph(out);
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
