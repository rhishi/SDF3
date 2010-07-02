/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3generate.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 14, 2005
 *
 *  Function        :   Free SDF graphs
 *
 *  History         :
 *      14-07-05    :   Initial version.
 *
 * $Id: sdf3generate.cc,v 1.2 2008/09/18 07:38:21 sander Exp $
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

#include "sdf3generate.h"
#include "../../csdf.h"
#include "sdf/sdf.h"

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

    // Graph
    bool stronglyConnected;
    bool acyclic;
    bool multigraph;
    uint period;
    
    // Actors
    uint nrActors;
    double avgInDegree;
    double varInDegree;
    double minInDegree;
    double maxInDegree;
    double avgOutDegree;
    double varOutDegree;
    double minOutDegree;
    double maxOutDegree;
    
    // Rate
    double avgRate;
    double varRate;
    double minRate;
    double maxRate;
    uint repetitionVectorSum;
    
    // Execution time
    bool execTime;    
    uint nrProcTypes;
    double mapChance;
    double avgExecTime;
    double varExecTime;
    double minExecTime;
    double maxExecTime;
    
    // State size
    bool stateSize;
    double avgStateSize;
    double varStateSize;
    double minStateSize;
    double maxStateSize;
    
    // Token size
    bool tokenSize;
    double avgTokenSize;
    double varTokenSize;
    double minTokenSize;
    double maxTokenSize;
    
    // Buffer size
    bool bufferSize;
    
    // Bandwidth requirement
    bool bandwidthRequirement;
    double avgBandwidth;
    double varBandwidth;
    double minBandwidth;
    double maxBandwidth;

    // Latency requirement
    bool latencyRequirement;
    double avgLatency;
    double varLatency;
    double minLatency;
    double maxLatency;
 
    // Throughput constraint
    bool throughputConstraint;
    uint autoConcurrencyDegree;
    double throughputScaleFactor;
    
    // Initial tokens
    double initialTokenProp;
    
    // IntegerMCM
    bool integerMCM;
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
    out << "Usage: " << TOOL << " [--settings <file> --output <file>]" << endl;
    out << "   --settings  <file>  settings for the graph generator (default: ";
    out << "sdf3.opt)" << endl;
    out << "   --output <file>     output file (default: stdout)" << endl;
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
    }
}

/**
 * parseSettingsFile ()
 * The function parses all settings from the file.
 */
void parseSettingsFile(CString module, CString type)
{
    CNode *actorsNode, *degreeNode, *rateNode, *structureNode, *graphNode;
    CNode *procsNode, *execTimeNode, *stateSizeNode, *tokenSizeNode;
    CNode *initialTokensNode, *graphPropertiesNode, *throughputNode;
    CNode *bandwidthNode, *latencyNode, *sdf3Node, *settingsNode;
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
    
    // Graph 
    graphNode = CGetChildNode(settingsNode, "graph");
    if (graphNode == NULL)
        throw CException("No graph element in settings.");

    // Period of the graph
    if (CHasAttribute(graphNode, "period"))
        settings.period = CGetAttribute(graphNode, "period");
        
    actorsNode = CGetChildNode(graphNode, "actors");
    degreeNode = CGetChildNode(graphNode, "degree");
    rateNode = CGetChildNode(graphNode, "rate");

    settings.nrActors = CGetAttribute(actorsNode, "nr");
    settings.avgInDegree = CGetAttribute(degreeNode, "avg");
    settings.varInDegree = CGetAttribute(degreeNode, "var");
    settings.minInDegree = CGetAttribute(degreeNode, "min");
    settings.maxInDegree = CGetAttribute(degreeNode, "max");
    settings.avgOutDegree = CGetAttribute(degreeNode, "avg");
    settings.varOutDegree = CGetAttribute(degreeNode, "var");
    settings.minOutDegree = CGetAttribute(degreeNode, "min");
    settings.maxOutDegree = CGetAttribute(degreeNode, "max");
    settings.avgRate = CGetAttribute(rateNode, "avg");
    settings.varRate = CGetAttribute(rateNode, "var");
    settings.minRate = CGetAttribute(rateNode, "min");
    settings.maxRate = CGetAttribute(rateNode, "max");
    
    // Sum of the repetition vector constrained?
    if (CHasAttribute(rateNode, "repetitionVectorSum"))
    {
        settings.repetitionVectorSum 
                = CGetAttribute(rateNode, "repetitionVectorSum");
    }
    
    // Graph structure
    if (CHasChildNode(graphNode, "structure"))
    {
        structureNode = CGetChildNode(graphNode, "structure");
        
        if (CHasAttribute(structureNode, "stronglyConnected"))
            if(CGetAttribute(structureNode, "stronglyConnected") 
                    == CString("true"))
                settings.stronglyConnected = true;
        
        if (CHasAttribute(structureNode, "acyclic"))
            if(CGetAttribute(structureNode, "acyclic") 
                    == CString("true"))
                settings.acyclic = true;

        if (CHasAttribute(structureNode, "multigraph"))
            if(CGetAttribute(structureNode, "multigraph") 
                    == CString("false"))
                settings.acyclic = false;
    }

    // Initial tokens
    if (CHasChildNode(graphNode, "initialTokens"))
    {
        initialTokensNode = CGetChildNode(graphNode, "initialTokens");

        settings.initialTokenProp = CGetAttribute(initialTokensNode, "prop");
    }
    
    // Graph properties
    if (CHasChildNode(settingsNode, "graphProperties"))
    {
        graphPropertiesNode = CGetChildNode(settingsNode, "graphProperties");

        if (CHasChildNode(graphPropertiesNode , "procs") 
                && CHasChildNode(graphPropertiesNode , "execTime"))
        {
            procsNode = CGetChildNode(graphPropertiesNode, "procs");
            execTimeNode = CGetChildNode(graphPropertiesNode, "execTime");

            settings.execTime = true;
            settings.nrProcTypes = CGetAttribute(procsNode, "nrTypes");
            settings.mapChance = CGetAttribute(procsNode, "mapChance");
            settings.avgExecTime = CGetAttribute(execTimeNode, "avg");
            settings.varExecTime = CGetAttribute(execTimeNode, "var");
            settings.minExecTime = CGetAttribute(execTimeNode, "min");
            settings.maxExecTime = CGetAttribute(execTimeNode, "max");
        }

        if (CHasChildNode(graphPropertiesNode , "stateSize"))
        {
            stateSizeNode = CGetChildNode(graphPropertiesNode, "stateSize");

            settings.stateSize = true;
            settings.avgStateSize = CGetAttribute(stateSizeNode, "avg");
            settings.varStateSize = CGetAttribute(stateSizeNode, "var");
            settings.minStateSize = CGetAttribute(stateSizeNode, "min");
            settings.maxStateSize = CGetAttribute(stateSizeNode, "max");
        }

        if (CHasChildNode(graphPropertiesNode, "tokenSize"))
        {
            tokenSizeNode = CGetChildNode(graphPropertiesNode, "tokenSize");

            settings.tokenSize = true;
            settings.avgTokenSize = CGetAttribute(tokenSizeNode, "avg");
            settings.varTokenSize = CGetAttribute(tokenSizeNode, "var");
            settings.minTokenSize = CGetAttribute(tokenSizeNode, "min");
            settings.maxTokenSize = CGetAttribute(tokenSizeNode, "max");
        }

        if (CHasChildNode(graphPropertiesNode, "bufferSize"))
            settings.bufferSize = true;

        if (CHasChildNode(graphPropertiesNode, "bandwidthRequirement"))
        {
            bandwidthNode = CGetChildNode(graphPropertiesNode,
                                                    "bandwidthRequirement");

            settings.bandwidthRequirement = true;
            settings.avgBandwidth = CGetAttribute(bandwidthNode, "avg");
            settings.varBandwidth = CGetAttribute(bandwidthNode, "var");
            settings.minBandwidth = CGetAttribute(bandwidthNode, "min");
            settings.maxBandwidth = CGetAttribute(bandwidthNode, "max");
        }

        if (CHasChildNode(graphPropertiesNode, "latencyRequirement"))
        {
            latencyNode = CGetChildNode(graphPropertiesNode,
                                                    "latencyRequirement");

            settings.latencyRequirement = true;
            settings.avgLatency = CGetAttribute(latencyNode, "avg");
            settings.varLatency = CGetAttribute(latencyNode, "var");
            settings.minLatency = CGetAttribute(latencyNode, "min");
            settings.maxLatency = CGetAttribute(latencyNode, "max");
        }
        
        if (CHasChildNode(graphPropertiesNode, "throughputConstraint"))
        {
            throughputNode = CGetChildNode(graphPropertiesNode,
                                                    "throughputConstraint");

            settings.throughputConstraint = true;
            
            if (CHasAttribute(throughputNode, "autoConcurrencyDegree"))
            {
                settings.autoConcurrencyDegree = CGetAttribute(throughputNode,
                                                    "autoConcurrencyDegree");
            }
            if (CHasAttribute(throughputNode, "scaleFactor"))
            {
                settings.throughputScaleFactor = CGetAttribute(throughputNode,
                                                    "scaleFactor");
            }
        }

        if (CHasChildNode(graphPropertiesNode, "integerMCM"))
            settings.integerMCM = true;
    }
}

/**
 * setDefaults ()
 * Set all settings at their default value.
 */
void setDefaults()
{
    settings.settingsFile = "sdf3.opt";
    settings.stronglyConnected = false;
    settings.acyclic = false;
    settings.multigraph = true;
    settings.nrActors = 0;
    settings.period = 1;
    settings.avgInDegree = 0;
    settings.varInDegree = 0;
    settings.minInDegree = 0;
    settings.maxInDegree = 0;
    settings.avgOutDegree = 0;
    settings.varOutDegree = 0;
    settings.minOutDegree = 0;
    settings.maxOutDegree = 0;
    settings.avgRate = 0;
    settings.varRate = 0;
    settings.minRate = 0;
    settings.maxRate = 0;
    settings.repetitionVectorSum = 0;
    settings.execTime = false;
    settings.nrProcTypes = 0;
    settings.mapChance = 0;
    settings.avgExecTime = 0;
    settings.varExecTime = 0;
    settings.minExecTime = 0;
    settings.maxExecTime = 0;
    settings.stateSize = false;
    settings.avgStateSize = 0;
    settings.varStateSize = 0;
    settings.minStateSize = 0;
    settings.maxStateSize = 0;
    settings.tokenSize = false;
    settings.avgTokenSize = 0;
    settings.varTokenSize = 0;
    settings.minTokenSize = 0;
    settings.maxTokenSize = 0;
    settings.bufferSize = false;
    settings.bandwidthRequirement = false;
    settings.avgBandwidth = 0;
    settings.varBandwidth = 0;
    settings.minBandwidth = 0;
    settings.maxBandwidth = 0; 
    settings.latencyRequirement = false;
    settings.avgLatency = 0;
    settings.varLatency = 0;
    settings.minLatency = 0;
    settings.maxLatency = 0; 
    settings.throughputConstraint = false;
    settings.autoConcurrencyDegree = 0;
    settings.throughputScaleFactor = 1;
    settings.initialTokenProp = 0.1;
    settings.integerMCM = false;
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
 * generateRandomSDFG ()
 * Generate a random SDF graph which is connected, consistent and deadlock-free.
 */
void generateRandomSDFG(ostream &out)
{
    TimedSDFgraph *sdfGraph;

    sdfGraph  = generateSDFgraph(settings.nrActors,
                settings.avgInDegree, settings.varInDegree,
                settings.minInDegree, settings.maxInDegree,
                settings.avgOutDegree, settings.varOutDegree, 
                settings.minOutDegree, settings.maxOutDegree, 
                settings.avgRate, settings.varRate,
                settings.minRate, settings.maxRate,
                settings.acyclic, settings.stronglyConnected,
                settings.initialTokenProp, settings.repetitionVectorSum,
                settings.multigraph);

    generateSDFgraphProperties(sdfGraph,
                settings.execTime, settings.nrProcTypes, 
                settings.mapChance, settings.avgExecTime, 
                settings.varExecTime, settings.minExecTime,
                settings.maxExecTime, settings.stateSize, 
                settings.avgStateSize, settings.varStateSize,
                settings.minStateSize, settings.maxStateSize,
                settings.tokenSize, settings.avgTokenSize,
                settings.varTokenSize, settings.minTokenSize,
                settings.maxTokenSize, settings.throughputConstraint,
                settings.autoConcurrencyDegree, settings.throughputScaleFactor,
                settings.bandwidthRequirement, settings.avgBandwidth,
                settings.varBandwidth, settings.minBandwidth,
                settings.maxBandwidth, settings.bufferSize,
                settings.latencyRequirement, settings.avgLatency,
                settings.varLatency, settings.minLatency,
                settings.maxLatency, settings.integerMCM);

    outputSDFasXML(sdfGraph, out);
}

/**
 * generateRandomCSDFG ()
 * Generate a random CSDF graph which is connected, consistent and
 * deadlock-free.
 */
void generateRandomCSDFG(ostream &out)
{
    TimedCSDFgraph *csdfGraph;

    csdfGraph  = generateCSDFgraph(settings.period, settings.nrActors,
                settings.avgInDegree, settings.varInDegree,
                settings.minInDegree, settings.maxInDegree,
                settings.avgOutDegree, settings.varOutDegree, 
                settings.minOutDegree, settings.maxOutDegree, 
                settings.avgRate, settings.varRate,
                settings.minRate, settings.maxRate,
                settings.acyclic, settings.stronglyConnected,
                settings.initialTokenProp, settings.repetitionVectorSum,
                settings.execTime, settings.nrProcTypes, 
                settings.mapChance, settings.avgExecTime, 
                settings.varExecTime, settings.minExecTime,
                settings.maxExecTime, settings.stateSize, 
                settings.avgStateSize, settings.varStateSize,
                settings.minStateSize, settings.maxStateSize,
                settings.tokenSize, settings.avgTokenSize,
                settings.varTokenSize, settings.minTokenSize,
                settings.maxTokenSize, settings.throughputConstraint,
                settings.autoConcurrencyDegree, settings.throughputScaleFactor,
                settings.bandwidthRequirement, settings.avgBandwidth,
                settings.varBandwidth, settings.minBandwidth,
                settings.maxBandwidth, settings.bufferSize,
                settings.latencyRequirement, settings.avgLatency,
                settings.varLatency, settings.minLatency,
                settings.maxLatency, settings.multigraph,
                settings.integerMCM);

    outputCSDFasXML(csdfGraph, out);
}

/**
 * generateRandomGraph ()
 * Generate a random (C)SDF graph which is connected, consistent and
 * deadlock-free.
 */
void generateRandomGraph(ostream &out)
{
    if (settings.period == 1)
        generateRandomSDFG(out);
    else
        generateRandomCSDFG(out);
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

        // Generate random graph
        generateRandomGraph(out);
    }
    catch (CException &e)
    {
        cerr << e;
        exit_status = 1;
    }

    return exit_status;
}
