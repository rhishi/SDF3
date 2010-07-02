/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   cost.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Communication scheduling tool
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: sdf3cost.cc,v 1.2 2008/03/20 16:16:21 sander Exp $
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

#include "sdf3cost.h"
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

    // Scheduler
    CString nocMappingAlgo;
    
    // Architecture graph
    CNode *xmlArchGraph;
    
    // System usage
    CNode *xmlSystemUsage;
    
    // Messages set (schedule problems)
    CNode *xmlMessagesSet;
    
    // Constraints
    uint maxDetour;
    uint maxNrRipups;
    uint maxNrTries;
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
    out << " --output <file>]";
    out << endl;
    out << "   --settings  <file>  settings for the scheduler (default: ";
    out << "sdf3.opt)" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
    out << "   --algo <algo>       scheduling algorithm:";
    out << endl;
    out << "       greedy (default)" << endl;              
    out << "       ripup" << endl;              
    out << "       knowledge" << endl;              
    out << "       random" << endl;              
    out << "       classic" << endl;              
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
        else if (argv[arg] == CString("--algo") && arg+1<argc)
        {
            arg++;
            settings.nocMappingAlgo = argv[arg];
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
 * loadMessagesSetFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the messages that must be scheduled on the
 * arhicteture.
 */
CNode *loadMessagesSetFromFile(CString &file, CString &module)
{
    CNode *messagesSetNode, *sdf3Node;
    CDoc *messagesSetDoc;
    
    // Open file
    messagesSetDoc = CParseFile(file);
    if (messagesSetDoc == NULL)
        throw CException("Failed loading system usage from '" + file + "'.");
    
    // Locate the sdf3 root element and check module type
    sdf3Node = CGetRootNode(messagesSetDoc);
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + file + "' is not "
                         "of type '" + module + "'.");
    }
    
    // Get messages set element
    messagesSetNode = CGetChildNode(sdf3Node, "messagesSet");
    if (messagesSetNode == NULL)
        throw CException("No messages set '" + file + "'.");
    
    return messagesSetNode;
}

/**
 * parseSettingsFile ()
 * Load all settings from the settings file.
 */
void parseSettingsFile(CString module, CString type)
{
    CNode *maxDetourNode, *maxNrRipupsNode, *maxNrTriesNode;
    CNode *messagesSetNode, *nocMappingNode, *systemUsageNode;
    CNode *constraintsNode, *settingsNode, *archGraphNode;
    CNode *sdf3Node;
    CString name, file;
    CDoc *settingsDoc;
    
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

    // Messages set
    messagesSetNode =  CGetChildNode(settingsNode, "messagesSet");
    if (messagesSetNode == NULL)
        throw CException("No messagesSet specified.");
    if (!CHasAttribute(messagesSetNode, "file"))
        throw CException("Missing file attribute on messagesSet.");
    file = CGetAttribute(messagesSetNode, "file");
    settings.xmlMessagesSet = loadMessagesSetFromFile(file, module);

    // NoC mapping algorithm
    nocMappingNode = CGetChildNode(settingsNode, "nocMapping");
    if (nocMappingNode != NULL)
    {
        if (CHasAttribute(nocMappingNode, "algo"))
            settings.nocMappingAlgo = CGetAttribute(nocMappingNode, "algo");
    }

    // Constrains used in NoC scheduling problem
    constraintsNode = CGetChildNode(nocMappingNode, "constraints");
    if (constraintsNode != NULL)
    {
        maxDetourNode = CGetChildNode(constraintsNode, "maxDetour");
        if (maxDetourNode != NULL)
            settings.maxDetour = (uint)CGetAttribute(maxDetourNode, "d");

        maxNrRipupsNode = CGetChildNode(constraintsNode, "maxNrRipups");
        if (maxNrRipupsNode != NULL)
            settings.maxNrRipups = (uint)CGetAttribute(maxNrRipupsNode, "n");

        maxNrTriesNode = CGetChildNode(constraintsNode, "maxNrTries");
        if (maxNrTriesNode != NULL)
            settings.maxNrTries = (uint)CGetAttribute(maxNrTriesNode, "n");
    }
}

/**
 * setDefaults ()
 * Set all settings at their default value.
 */
void setDefaults()
{
    // Settings file
    settings.settingsFile = "sdf3.opt";
    
    // Output (stdout)
    settings.outputFile = "";
    
    // Architecture graph
    settings.xmlArchGraph = NULL;
    settings.xmlSystemUsage = NULL;

    // NoC mapping algorithm
    settings.nocMappingAlgo = "greedy";
    
    // Constrains used in NoC scheduling problem
    settings.maxDetour = 0;
    settings.maxNrRipups = 0;
    settings.maxNrTries = 0;
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

    // Parse settings
    parseSettingsFile(MODULE, SETTINGS_TYPE);
}

/**
 * outputNetworkUsageAsXML ()
 * Output the usage of the interconnect as XML.
 */
void outputNetworkUsageAsXML(SetOfNoCScheduleProblems &problems, ostream &out)
{
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                            "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                            "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");
    
    // Mapping node
    CNode *usageNode = CAddNode(sdf3Node, "systemUsage");
    CAddNode(usageNode, problems.createNetworkUsageNode());
    
    // Create document and save it
    CDoc *doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);
}

/**
 * outputNetworkBindingAsXML ()
 * Output the binding of messages to the interconnect as XML.
 */
void outputNetworkBindingAsXML(SetOfNoCScheduleProblems &problems, ostream &out)
{
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                            "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                            "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");
    
    // Mapping node
    CNode *mappingNode = CAddNode(sdf3Node, "mapping");
    CAddNode(mappingNode, problems.createNetworkMappingNode());
    
    // Create document and save it
    CDoc *doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);
}

/**
 * solveSchedulingProblems ()
 * The function constructs a scheduling problem and tries to solve it using the
 * requested scheduler.
 */
bool solveSchedulingProblems(ostream &out)
{
    SetOfNoCScheduleProblems *problems;
    NoCScheduler *scheduler;
    bool solved = false;
    CTimer timer;

    // Create a scheduler for the scheduling problem
    if (settings.nocMappingAlgo == "greedy")
    {
        scheduler = new GreedyNoCScheduler(settings.maxDetour);
    }
    else if (settings.nocMappingAlgo == "ripup")
    {
        scheduler = new RipupNoCScheduler(settings.maxDetour,
                                                settings.maxNrRipups);
    }
    else if (settings.nocMappingAlgo == "knowledge")
    {
        scheduler = new KnowledgeNoCScheduler(settings.maxDetour,
                                                settings.maxNrRipups);
    }
    else if (settings.nocMappingAlgo == "random")
    {
        scheduler = new RandomNoCScheduler(settings.maxDetour,
                                settings.maxNrRipups, settings.maxNrTries);
    }
    else if (settings.nocMappingAlgo == "classic")
    {
        scheduler = new ClassicNoCScheduler(settings.maxDetour,
                                                settings.maxNrRipups);
    }
    else
    {
        throw CException("[ERROR] Unknown scheduling algorithm.");
    }

    // Create a set of scheduling problems
    problems = new SetOfNoCScheduleProblems(settings.xmlMessagesSet, 
                                settings.xmlArchGraph, settings.xmlSystemUsage);

    // Measure time of scheduling algorithm
    startTimer(&timer);

    // Try to solve the scheduling problems
    solved = scheduler->schedule(*problems);

    // Measure time of scheduling algorithm
    stopTimer(&timer);
    cerr << "Time: ";
    printTimer(cerr, &timer);
    cerr << endl;
    
    // Output resulting mapping if problem is solved
    if (solved)
    {
        outputNetworkBindingAsXML(*problems, out);
        outputNetworkUsageAsXML(*problems, out);
    }

    return solved;
}

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    int exit_status = 1;
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

        // Schedule the all sets of messages
        if (solveSchedulingProblems(out))
            exit_status = 0;
    }
    catch (CException &e)
    {
        cerr << e;
    }

    return exit_status;
}
