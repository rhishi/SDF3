/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   gast.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 4, 2006
 *
 *  Function        :   SDF graph assignment tool
 *
 *  History         :
 *      04-04-06    :   Initial version.
 *
 * $Id: sdf3gast.cc,v 1.4 2008/03/06 10:49:45 sander Exp $
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

#include "sdf3gast.h"
#include "base/base.h"
#include "../../sdf.h"

/**
 * Settings
 * Struct to store program settings.
 */
typedef struct _Settings
{
    // settings file
    CString settingsFile;
    
    // output file
    CString outputFile;

    // Binding algorithm
    CString tileMappingAlgo;
    
    // Architecture graph
    CNode *xmlArchGraph;
    CNode *xmlSystemUsage;
    
    // Application graphs
    list<CNode*> xmlAppGraphs;
    
    // Bindings
    CNode *xmlMapping;
    
    // Constants used in tile cost function
    double cnst_a, cnst_b, cnst_c, cnst_d, cnst_e, cnst_f, cnst_g;
    double cnst_k, cnst_l, cnst_m, cnst_n, cnst_o, cnst_p, cnst_q;
    
    // Run only binding step
    bool check;
    
    // Communication trace
    bool communicationTrace;
    uint slotTableSize;
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
    out << "Usage: " << TOOL << " [--settings <file> --scheduler <algo>";
    out << " --output <file>" << endl;
    out << "                     --check --stateSpace --communicationTrace]";
    out << endl;
    out << "   --settings  <file>         settings for tool (default: ";
    out << "sdf3.opt)" << endl;
    out << "   --output <file>            output file (default: stdout)";
    out << endl;
    out << "   --algo <algo>              binding algorithm:";
    out << endl;
    out << "       loadbalance (default)" << endl;
    out << "   --check                    run only binding step" << endl;
    out << "   --communicationTrace <sz>  give interconnect communication";
    out << " for specified" << endl;
    out << "                              slot table size";
    out << endl;
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
        if (argv[arg] == CString("--output") && arg+1<argc)
        {
            arg++;
            settings.outputFile = argv[arg];
        }
        else if (argv[arg] == CString("--bindingAlgo") && arg+1<argc)
        {
            arg++;
            settings.tileMappingAlgo = argv[arg];
        }
        else if (argv[arg] == CString("--check"))
        {
            settings.check = true;
        }
        else if (argv[arg] == CString("--communicationTrace") && arg+1<argc)
        {
            arg++;
            settings.communicationTrace = true;
            settings.slotTableSize = CString(argv[arg]);
        }
        else if (argv[arg] == CString("--settings") && arg+1<argc)
        {
            arg++;
            settings.settingsFile = argv[arg];
        }
        else
        {
            helpMessage(cerr);
            throw CException("");
        }
        
        // Next argument
        arg++;
    };
}

/**
 * loadApplicationGraphFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the SDFG.
 */
CNode *loadApplicationGraphFromFile(CString &file, CString &module)
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
 * supplied file that describes the platform.
 */
CNode *loadArchitectureGraphFromFile(CString &file, CString &module)
{
    CNode *archGraphNode, *sdf3Node;
    CDoc *archGraphDoc;
    
    // Open file
    archGraphDoc = CParseFile(file);
    if (archGraphDoc == NULL)
        throw CException("Failed loading architecture from '" + file + "'.");

    // Locate the sdf3 root element and check module type
    sdf3Node = CGetRootNode(archGraphDoc);
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + file + "' is not "
                         "of type '" + module + "'.");
    }
    
    // Get architecture graph node
    archGraphNode = CGetChildNode(sdf3Node, "architectureGraph");
    if (archGraphNode == NULL)
        throw CException("No architecture graph in '" + file + "'.");
    
    return archGraphNode;
}

/**
 * loadSystemUsageFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the used resources by other applications.
 */
CNode *loadSystemUsageFromFile(CString &file, CString &module)
{
    CNode *systemUsageNode, *sdf3Node;
    CDoc *systemUsageDoc;
    
    // Open file
    systemUsageDoc = CParseFile(file);
    if (systemUsageDoc == NULL)
        throw CException("Failed loading system usage from '" + file + "'.");
    
    // Locate the sdf3 root element and check module type
    sdf3Node = CGetRootNode(systemUsageDoc);
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + file + "' is not "
                         "of type '" + module + "'.");
    }
    
    // Get system usage element
    systemUsageNode = CGetChildNode(sdf3Node, "systemUsage");
    if (systemUsageNode == NULL)
        throw CException("No system usage information in '" + file + "'.");
    
    return systemUsageNode;
}

/**
 * parseSettingsFile ()
 * The function parses all settings from the file.
 */
void parseSettingsFile(CString module, CString type)
{
    CNode *appGraphNode, *constantsNode, *constantNode;
    CNode *settingsNode, *archGraphNode, *systemUsageNode;
    CNode *sdf3Node, *tileMappingNode;
    CString name, file;
    CDoc *settingsDoc;
    double value;
    
    // Open settings file and get root node
    settingsDoc = CParseFile(settings.settingsFile);
    sdf3Node = CGetRootNode(settingsDoc);
    if (sdf3Node == NULL)
    {
        throw CException("Failed opening '" + settings.settingsFile + "'.");
    }
    
    // Is the node of the correct type?
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + settings.settingsFile 
                         + "' is not of type '" + module + "'.");
    }
    
    // Get the settings element of the tool
    for (settingsNode = CGetChildNode(sdf3Node, "settings");
            settingsNode != NULL; 
                settingsNode = CNextNode(settingsNode, "settings"))
    {
        if (CGetAttribute(settingsNode, "type") == type)
        {
            break;
        }
    }
    
    // Found the correct settings element?
    if (settingsNode == NULL)
    {
        throw CException("File '" + settings.settingsFile + "' contains no "
                         " settings of type '" + type + "'.");
    }

    // Application graphs
    appGraphNode =  CGetChildNode(settingsNode, "applicationGraph");
    while (appGraphNode != NULL)
    {
        // Has file attribute?
        if (!CHasAttribute(appGraphNode, "file"))
            throw CException("Missing file attribute on applicationGraph.");
        
        // Load the application from the specified file
        file = CGetAttribute(appGraphNode, "file");
        settings.xmlAppGraphs.push_back(
                            loadApplicationGraphFromFile(file, module));
        
        // Next application graph
        appGraphNode =  CNextNode(appGraphNode, "applicationGraph");
    }
    
    // At least one application is needed
    if (settings.xmlAppGraphs.size() == 0)
        throw CException("No applicationGraph specified.");

    // Architecture graph
    archGraphNode =  CGetChildNode(settingsNode, "architectureGraph");
    if (archGraphNode == NULL)
        throw CException("No architectureGraph specified.");
    if (!CHasAttribute(archGraphNode, "file"))
        throw CException("Missing file attribute on architectureGraph.");
    file = CGetAttribute(archGraphNode, "file");
    settings.xmlArchGraph = loadArchitectureGraphFromFile(file, module);

    // System usage information
    systemUsageNode =  CGetChildNode(settingsNode, "systemUsage");
    if (systemUsageNode != NULL)
    {
        if (!CHasAttribute(systemUsageNode, "file"))
            throw CException("Missing file attribute on systemUsage.");
        file = CGetAttribute(systemUsageNode, "file");
        settings.xmlSystemUsage = loadSystemUsageFromFile(file, module);
    }

    // Tile mapping algorithm
    tileMappingNode = CGetChildNode(settingsNode, "tileMapping");
    if (tileMappingNode != NULL)
    {
        if (CHasAttribute(tileMappingNode, "algo"))
            settings.tileMappingAlgo = CGetAttribute(tileMappingNode, "algo");
    }
    
    // Constants used in tile cost function
    constantsNode = CGetChildNode(tileMappingNode, "constants");
    if (constantsNode != NULL)
    {
        for (constantNode = CGetChildNode(constantsNode, "constant");
                constantNode != NULL; 
                    constantNode = CNextNode(constantNode, "constant"))
        {
            name = CGetAttribute(constantNode, "name");
            value = CGetAttribute(constantNode, "value");
            
            if (name == "a")
                settings.cnst_a = value;
            else if (name == "b")
                settings.cnst_b = value;
            else if (name == "c")
                settings.cnst_c = value;
            else if (name == "d")
                settings.cnst_d = value;
            else if (name == "e")
                settings.cnst_e = value;
            else if (name == "f")
                settings.cnst_f = value;
            else if (name == "g")
                settings.cnst_g = value;
            else if (name == "k")
                settings.cnst_k = value;
            else if (name == "l")
                settings.cnst_l = value;
            else if (name == "m")
                settings.cnst_m = value;
            else if (name == "n")
                settings.cnst_n = value;
            else if (name == "o")
                settings.cnst_o = value;
            else if (name == "p")
                settings.cnst_p = value;
            else if (name == "q")
                settings.cnst_q = value;
        }
    }
}

/**
 * setDefaults ()
 * Set all settings at their default value.
 */
void setDefaults()
{
    // settings file
    settings.settingsFile = "sdf3.opt";
    
    // Binding algorithm
    settings.tileMappingAlgo = "loadbalance";
    
    // Architecture graph
    settings.xmlArchGraph = NULL;
    settings.xmlSystemUsage = NULL;
    
    // Bindings
    settings.xmlMapping = NULL;
    
    // Constants used in tile cost function
    settings.cnst_a = 1;
    settings.cnst_b = settings.cnst_c = settings.cnst_d = settings.cnst_e = 0;
    settings.cnst_f = settings.cnst_g = 0;
    settings.cnst_k = settings.cnst_l = settings.cnst_m = settings.cnst_n = 1;
    settings.cnst_o = settings.cnst_p = settings.cnst_q = 1;
    
    // Run only binding step
    settings.check = false;
    
    // Communication trace
    settings.communicationTrace = false;
    settings.slotTableSize = 0;
}

/**
 * initSettings ()
 * The function initializes the program settings.
 */
void initSettings(int argc, char **argv)
{
    // Defaults
    setDefaults();
    
    // Parse the command line
    parseCommandLine(argc, argv);

    // Parse settings file
    parseSettingsFile(MODULE, SETTINGS_TYPE);
}

/**
 * outputSystemUsage ()
 * The function outputs the percentage of occupied resources for all elements in
 * the platform graph.
 */
void outputSystemUsage(ostream &out, PlatformGraph *g)
{
    for (TilesIter iter = g->tilesBegin(); iter != g->tilesEnd(); iter++)
    {
        Tile *t = *iter;
        Processor *p = t->getProcessor();
        Memory *m = t->getMemory();
        NetworkInterface *ni = t->getNetworkInterface();
        double perc;
        
        out << "Tile: " << t->getName() << endl;
        
        // Processor
        if (p != NULL)
        {
            perc = 100.0 * (p->getTimewheelSize() - p->availableTimewheelSize())
                                               / (double) p->getTimewheelSize();
            out << "    Timewheel:     " << perc << "% occupied" << endl;
        }   

        // Memory
        if (m != NULL)
        {
            perc = 100.0 * (m->getSize() - m->availableMemorySize())
                                              / (double) m->getSize();
            out << "    Memory:        " << perc << "% occupied" << endl;
        }   

        // Network interface
        if (ni != NULL)
        {
            perc = 100.0*(ni->getNrConnections() - ni->availableNrConnections())
                                              / (double) ni->getNrConnections();
            out << "    Connections:   " << perc << "% occupied" << endl;

            perc = 100.0 * (ni->getInBandwidth() - ni->availableInBandwidth())
                                              / (double) ni->getInBandwidth();
            out << "    In bandwidth:  " << perc << "% occupied" << endl;

            perc = 100.0 * (ni->getOutBandwidth() - ni->availableOutBandwidth())
                                              / (double) ni->getOutBandwidth();
            out << "    Out bandwidth: " << perc << "% occupied" << endl;
        }   
    }
}

/**
 * outputCommunicationTrace ()
 * The function outputs all message sets (i.e. the communication scheduling
 * problem) to the supplied stream.
 */
void outputCommunicationTrace(TimedSDFgraph *appGraph, PlatformGraph *archGraph,
        ostream &out)
{
    SDFstateSpaceTraceInterconnectCommunication traceCommunication;
    BindingAwareSDFG *bindingAwareSDFG;
    CNode *messagesSetNode, *sdf3Node;
    CDoc *doc;
    
    // Create a binding-aware SDFG
    bindingAwareSDFG = new BindingAwareSDFG(appGraph,archGraph,SDFflowTypeNSoC);

    // Trace all messages sent over the interconnect
    messagesSetNode = traceCommunication.trace(bindingAwareSDFG, archGraph,
                                                        settings.slotTableSize);

    // SDF mapping node
    sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                            "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                            "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");
    
    // Messages sets
    CAddNode(sdf3Node, messagesSetNode);
    
    // Create document and save it
    doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);

    // Cleanup
    delete bindingAwareSDFG;
}

/**
 * bindApplicationGraphsToArchitectureGraph ()
 * The function tries to bind all application graphs, in the order specified in
 * the XML file, to the architecture graph using the requested binding
 * algorithm. When an application is succesfully bound to the architecture, it
 * outputs the binding.
 * The function returns true if all application graphs are succesfully bound to
 * the architecture graph. Otherwise, it returns false.
 */
bool bindApplicationGraphsToArchitectureGraph(ostream &out, bool check = false)
{
    CNode *systemUsageNode, *appGraphNode, *sdfGraphNode, *sdfGraphPropsNode;
    LoadBalanceBinding loadBalanceBinding(SDFflowTypeNSoC);
    bool setBinding, valid = true;
    TimedSDFgraph *appGraph;
    PlatformGraph *archGraph;
    uint nrAppGraphsBound = 0;
    CTimer timer1, timer2;
    
    // Set initial resource usage information
    systemUsageNode = settings.xmlSystemUsage;
    
    // Set constants tile cost function
    loadBalanceBinding.setConstantsTileCostFunction(
                        settings.cnst_a, settings.cnst_b, settings.cnst_c,
                        settings.cnst_d, settings.cnst_e, settings.cnst_f,
                        settings.cnst_g, settings.cnst_k, settings.cnst_l,
                        settings.cnst_m, settings.cnst_n, settings.cnst_o,
                        settings.cnst_p, settings.cnst_q);
    
    // Timer 1: Total time spent on resource allocation
    startTimer(&timer1);
    
    // Iterate over all application graphs
    for (list<CNode*>::iterator appsIter = settings.xmlAppGraphs.begin();
            appsIter != settings.xmlAppGraphs.end(); appsIter++)
    {
        appGraphNode = *appsIter;
        
        // No binding found for the application at this moment
        setBinding = false;
        
        // Application graph
        sdfGraphNode = CGetChildNode(appGraphNode, "sdf");
        sdfGraphPropsNode = CGetChildNode(appGraphNode, "sdfProperties");

        // Construct application graph
        appGraph = new TimedSDFgraph();
        appGraph->construct(sdfGraphNode, sdfGraphPropsNode);

        // Construct architecture graph
        archGraph = constructPlatformGraph(settings.xmlArchGraph);

        // Mark all resource occupied by earlier mapped graphs as unavailable
        if (check == false)
            setUsagePlatformGraph(archGraph, systemUsageNode);

        // Binding specified for this application and architecture?
        for (CNode *mappingNode = settings.xmlMapping;
                mappingNode != NULL; 
                    mappingNode = CNextNode(mappingNode, "mapping"))
        {
            if (archGraph->getName() == CGetAttribute(mappingNode, "archGraph")
               && appGraph->getName() == CGetAttribute(mappingNode, "appGraph"))
            {
                setMappingPlatformGraph(archGraph, appGraph, mappingNode);
                setBinding = true;
                valid = true;
            }
        }

        // No binding found yet?
        if (!setBinding)
        {
            // Timer 2: resource allocation of single graph
            startTimer(&timer2);

            // Run selected binding algorithm
            if (settings.tileMappingAlgo == "loadbalance")
            {
                loadBalanceBinding.setAppGraph(appGraph);
                loadBalanceBinding.setArchGraph(archGraph);
                if (check)
                {
                    valid = loadBalanceBinding.bindingCheck();
                }
                else
                {
                    valid = loadBalanceBinding.bindSDFGtoTiles();
                    if (valid)
                    {
                        valid = loadBalanceBinding.
                                            constructStaticOrderScheduleTiles();
                    }
                    if (valid)
                    {
                        valid = loadBalanceBinding.allocateTDMAtimeSlices();
                    }

                }
            }
            else
            {
                throw CException("Binding algorithm unknown.");
            }

            // Timer 2
            stopTimer(&timer2);
            cerr << "Resource allocation took ";
            printTimer(cerr, &timer2);
            cerr << endl;
        }
        
        // Found no valid binding?
        if (!valid)
        {
            outputSystemUsageAsXML(archGraph, out);

            cerr << "Failed binding '" << CGetAttribute(appGraphNode, "name");
            cerr << "'" << endl;
            cerr << "[INFO] Bound " << nrAppGraphsBound;
            cerr << " applications to architecture." << endl;
            
            outputSystemUsage(cerr, archGraph);

            // Resource usage
            systemUsageNode = createSystemUsageNode(archGraph);
            
            // Stop binding applications to architecture
            break;
        }
        else
        {
            // Bound application to architecture
            nrAppGraphsBound++;

            // Output binding
            outputBindingAsXML(archGraph, appGraph, out);
            out.flush();
            
            // Resource usage
            systemUsageNode = createSystemUsageNode(archGraph);

            cerr << "[INFO] Bound " << nrAppGraphsBound;
            cerr << " applications to architecture." << endl;

            outputSystemUsage(cerr, archGraph);
            
            // Communication trace requested?
            if (settings.communicationTrace)
            {
                cerr << "[INFO] Extracting interconnect communication." << endl;
                outputCommunicationTrace(appGraph, archGraph, out);
            }
        }

        // Cleanup
        delete archGraph;
        delete appGraph;
    }

    // Timer 1
    stopTimer(&timer1);
    cerr << "Total resource allocation took ";
    printTimer(cerr, &timer1);
    cerr << endl;

    cerr << "[INFO] Bound " << nrAppGraphsBound;
    cerr << " applications to architecture." << endl;

    return valid;
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

        // Run binding algorithm
        if (bindApplicationGraphsToArchitectureGraph(out, settings.check))
            exit_status = 0;
        else
            exit_status = 1;
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
