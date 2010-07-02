/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   settings.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 24, 2007
 *
 *  Function        :   Settings for sdf3flow
 *
 *  History         :
 *      24-07-07    :   Initial version.
 *
 * $Id: settings.cc,v 1.4 2008/03/20 16:16:21 sander Exp $
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

#include "settings.h"

/**
 * Settings ()
 * Constructor. Create settings object with default values.
 */
Settings::Settings(CString module, CString type)
    : module(module), type(type)
{
    // No help requested
    helpFlag = false;
    
    // Settings file
    settingsFile = "sdf3.opt";
    
    // Output (stdout)
    outputFile = "";
    
    // Run flow step-by-step
    stepFlag = false;

    // Output results as HTML
    outputAsHTML = false;

    // Flow type
    flowType = SDFflowTypeNSoC;

    // Architecture graph
    xmlArchGraph = NULL;
    xmlSystemUsage = NULL;

    // Tile mapping algorithm
    tileMappingAlgo = "loadbalance";

    // NoC mapping algorithm
    nocMappingAlgo = "greedy";
    
    // Constants used in tile cost function
    cnst_a = 1;
    cnst_b = cnst_c = cnst_d = cnst_e = cnst_f = cnst_g = 0;
    cnst_k = cnst_l = cnst_m = cnst_n = cnst_o = cnst_p = cnst_q = 1;
    
    // Constrains used in NoC scheduling problem
    maxDetour = 0;
    maxNrRipups = 0;
    maxNrTries = 0;
}

/**
 * ~Settings ()
 * Destructor.
 */
Settings::~Settings()
{
}

/**
 * init ()
 * Initialize the settings using the supplied arguments. Note that the first
 * argument (argv[0]) is skipped.
 */
void Settings::init(int argc, char **argv)
{
    CStrings args;
    
    for (int i = 1; i < argc; i++)
        args.push_back(argv[i]);
    
    init(args);
}

/**
 * init ()
 * Initialize the settings using the supplied arguments. The function
 * loads also external files (e.g. settings, graph, etc) when required.
 */
void Settings::init(CStrings args)
{
    // Parse list of arguments
    parseArguments(args);
    
    // Only continue when no help is needed
    if (helpFlag)
        return;
    
    // Parse settings file
    parseSettingsFile();
}

/**
 * parseArguments ()
 * Parse the supplied arguments.
 */
void Settings::parseArguments(CStrings args)
{
    CStringsIter argIter = args.begin();

    // Parse list of arguments
    while (argIter != args.end())
    {
        CString arg, argNext;
        
        // Current argument
        arg = *argIter;
        
        // Argument left in list of arguments?
        if (argIter != args.end())
        {
            argIter++;
            if (argIter != args.end())
                argNext = *argIter;
        }
        
        // Parse argument
        if (arg == "--settings")
        {
            settingsFile = argNext;
            argIter++;
        }
        else if (arg == "--output")
        {
            outputFile = argNext;
            argIter++;
        }
        else if (arg == "--step")
        {
            stepFlag = true;
        }
        else if (arg == "--html")
        {
            outputAsHTML = true;
        }
        else
        {
            helpFlag = true;
        }
    }
}

/**
 * parseSettingsFile ()
 * Load all settings from the settings file.
 */
void Settings::parseSettingsFile()
{
    CNode *maxDetourNode, *maxNrRipupsNode, *maxNrTriesNode;
    CNode *tileMappingNode, *nocMappingNode, *systemUsageNode;
    CNode *constantsNode, *constantNode, *constraintsNode;
    CNode *settingsNode, *archGraphNode, *appGraphNode;
    CNode *sdf3Node, *flowTypeNode;
    CString name, file, flowTypeString;
    CDoc *settingsDoc;
    double value;
    
    // Open settings file and get root node
    settingsDoc = CParseFile(settingsFile);
    sdf3Node = CGetRootNode(settingsDoc);
    if (sdf3Node == NULL)
    {
        throw CException("Failed opening '" + settingsFile + "'.");
    }
    
    // Is the node of the correct type?
    if (CGetAttribute(sdf3Node, "type") != module)
    {
        throw CException("Root element in file '" + settingsFile + "' is not "
                         "of type '" + module + "'.");
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
        throw CException("File '" + settingsFile + "' contains no settings " +
                         "of type '" + type + "'.");
    }

    // Flow type
    flowTypeNode = CGetChildNode(settingsNode, "flowType");
    if (flowTypeNode != NULL)
    {
        if (!CHasAttribute(flowTypeNode, "type"))
            throw CException("Missing type attribute on flowType.");
        flowTypeString = CGetAttribute(flowTypeNode, "type");
        if (flowTypeString ==  "NSoC")
        {
                flowType = SDFflowTypeNSoC;
        }
        else if (flowTypeString ==  "MPFlow")
        {
                flowType = SDFflowTypeMPFlow;
        }
        else
        {
            throw CException("Flow type '" + flowTypeString 
                              + "'is not supported.");
        }
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
        xmlAppGraphs.push_back(loadApplicationGraphFromFile(file));
        
        // Next application graph
        appGraphNode =  CNextNode(appGraphNode, "applicationGraph");
    }
    
    // At least one application is needed
    if (xmlAppGraphs.size() == 0)
        throw CException("No applicationGraph specified.");

    // Architecture graph
    archGraphNode =  CGetChildNode(settingsNode, "architectureGraph");
    if (archGraphNode == NULL)
        throw CException("No architectureGraph specified.");
    if (!CHasAttribute(archGraphNode, "file"))
        throw CException("Missing file attribute on architectureGraph.");
    file = CGetAttribute(archGraphNode, "file");
    xmlArchGraph = loadArchitectureGraphFromFile(file);
    
    // System usage information
    systemUsageNode =  CGetChildNode(settingsNode, "systemUsage");
    if (systemUsageNode != NULL)
    {
        if (!CHasAttribute(systemUsageNode, "file"))
            throw CException("Missing file attribute on systemUsage.");
        file = CGetAttribute(systemUsageNode, "file");
        xmlSystemUsage = loadSystemUsageFromFile(file);
    }

    // Tile mapping algorithm
    tileMappingNode = CGetChildNode(settingsNode, "tileMapping");
    if (tileMappingNode != NULL)
    {
        if (CHasAttribute(tileMappingNode, "algo"))
            tileMappingAlgo = CGetAttribute(tileMappingNode, "algo");
    }
    
    // NoC mapping algorithm
    nocMappingNode = CGetChildNode(settingsNode, "nocMapping");
    if (nocMappingNode != NULL)
    {
        if (CHasAttribute(nocMappingNode, "algo"))
            nocMappingAlgo = CGetAttribute(nocMappingNode, "algo");
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
                cnst_a = value;
            else if (name == "b")
                cnst_b = value;
            else if (name == "c")
                cnst_c = value;
            else if (name == "d")
                cnst_d = value;
            else if (name == "e")
                cnst_e = value;
            else if (name == "f")
                cnst_f = value;
            else if (name == "g")
                cnst_g = value;
            else if (name == "k")
                cnst_k = value;
            else if (name == "l")
                cnst_l = value;
            else if (name == "m")
                cnst_m = value;
            else if (name == "n")
                cnst_n = value;
            else if (name == "o")
                cnst_o = value;
            else if (name == "p")
                cnst_p = value;
            else if (name == "q")
                cnst_q = value;
        }
    }

    // Constrains used in NoC scheduling problem
    constraintsNode = CGetChildNode(nocMappingNode, "constraints");
    if (constraintsNode != NULL)
    {
        maxDetourNode = CGetChildNode(constraintsNode, "maxDetour");
        if (maxDetourNode != NULL)
            maxDetour = (uint)CGetAttribute(maxDetourNode, "d");

        maxNrRipupsNode = CGetChildNode(constraintsNode, "maxNrRipups");
        if (maxNrRipupsNode != NULL)
            maxNrRipups = (uint)CGetAttribute(maxNrRipupsNode, "n");

        maxNrTriesNode = CGetChildNode(constraintsNode, "maxNrTries");
        if (maxNrTriesNode != NULL)
            maxNrTries = (uint)CGetAttribute(maxNrTriesNode, "n");
    }
}

/**
 * initOutputStream ()
 * Initialize the output stream.
 */
void Settings::initOutputStream()
{
    // Set output stream
    if (!outputFile.empty())   
        outputStream.open(outputFile.c_str());
    else
        ((ostream&)(outputStream)).rdbuf(cout.rdbuf());
}

/**
 * loadApplicationGraphFromFile ()
 * The function returns a pointer to an XML data structures contained in the
 * supplied file that describes the SDFG.
 */
CNode *Settings::loadApplicationGraphFromFile(CString &file)
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
CNode *Settings::loadArchitectureGraphFromFile(CString &file)
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
CNode *Settings::loadSystemUsageFromFile(CString &file)
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
