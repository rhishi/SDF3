/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sdf3flow.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   February 7, 2007
 *
 *  Function        :   SDFG mapping to MP-SoC
 *
 *  History         :
 *      07-02-07    :   Initial version.
 *
 * $Id: sdf3flow.cc,v 1.5 2008/05/07 11:29:38 sander Exp $
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

#include "sdf3flow.h"
#include "settings.h"
#include "../../sdf.h"

/**
 * settings
 * Program settings.
 */
Settings settings(MODULE, SETTINGS_TYPE);

/**
 * helpMessage ()
 * Function prints help message for the tool.
 */
void helpMessage(ostream &out)
{
    out << "SDF3 " << TOOL << " (version " << DOTTED_VERSION ")" << endl;
    out << endl;
    out << "Usage: " << TOOL << " [--settings <file> --output <file>";
    out << " --step]";
    out << endl;
    out << "   --settings <file>  settings for algorithms (default: sdf3.opt)";
    out << endl;
    out << "   --output <file>    output file (default:stdout)" << endl;
    out << "   --step             run flow step-by-step" << endl;
    out << "   --html             output result of flow in HTML" << endl;
    out << endl;
}

/**
 * initSettings ()
 * The function initializes the program settings.
 */
bool initSettings(int argc, char **argv)
{
    // Initialize settings
    settings.init(argc, argv);
    
    // Help message needed?
    if (settings.helpFlag)
    {
        helpMessage(cerr);
        return false;    
    }
    
    // Initialize output stream
    settings.initOutputStream();
    
    // Done
    return true;
}

/**
 * mapApplicationGraphToArchitectureGraph ()
 * The complete mapping flow. The function returns the system usage
 * after the mapping is completed. On failure, it returns NULL.
 */
CNode *mapApplicationGraphToArchitectureGraph(CNode *xmlAppGraph,
        CNode *xmlArchGraph, CNode *xmlSystemUsage, ostream &out, 
        bool outputAsHTML)
{
    NoCScheduler *nocMappingAlgo;
    TileMapping *tileMapping;
    NoCMapping *nocMapping;
    CNode *xmlSDF3Node;
    SDF3Flow::FlowState status;
    SDF3Flow *flow;
    CTimer timer;
    
    // Create a new mapping flow
    flow = new SDF3Flow(settings.flowType, xmlAppGraph, xmlArchGraph, 
                        xmlSystemUsage);

    // Run flow step-by-step?
    if (settings.stepFlag)
        flow->setStepMode(true);
    
    // Tile binding and scheduling algorithm
    if (settings.tileMappingAlgo == "loadbalance")
    {
        tileMapping = new LoadBalanceBinding(flow->getFlowType());
        ((LoadBalanceBinding*)(tileMapping))->setConstantsTileCostFunction(
                        settings.cnst_a, settings.cnst_b, settings.cnst_c,
                        settings.cnst_d, settings.cnst_e, settings.cnst_f,
                        settings.cnst_g, settings.cnst_k, settings.cnst_l,
                        settings.cnst_m, settings.cnst_n, settings.cnst_o,
                        settings.cnst_p, settings.cnst_q);
    }
    else
    {
        throw CException("[ERROR] Unknown tile mapping algorithm.");
    }
    flow->setTileMappingAlgo(tileMapping);
    
    // NoC routing and scheduling algorithm
    if (settings.nocMappingAlgo == "greedy")
    {
        nocMappingAlgo = new GreedyNoCScheduler(settings.maxDetour);
    }
    else if (settings.nocMappingAlgo == "ripup")
    {
        nocMappingAlgo = new RipupNoCScheduler(settings.maxDetour,
                                                settings.maxNrRipups);
    }
    else if (settings.nocMappingAlgo == "knowledge")
    {
        nocMappingAlgo = new KnowledgeNoCScheduler(settings.maxDetour,
                                                settings.maxNrRipups);
    }
    else if (settings.nocMappingAlgo == "random")
    {
        nocMappingAlgo = new RandomNoCScheduler(settings.maxDetour,
                                settings.maxNrRipups, settings.maxNrTries);
    }
    else if (settings.nocMappingAlgo == "classic")
    {
        nocMappingAlgo = new ClassicNoCScheduler(settings.maxDetour,
                                                settings.maxNrRipups);
    }
    else
    {
        throw CException("[ERROR] Unknown scheduling algorithm.");
    }
    nocMapping = new NoCMapping(nocMappingAlgo, flow->getFlowType());
    flow->setNoCMappingAlgo(nocMapping);

    // Measure execution time
    startTimer(&timer);
    
    // Execute the flow
    status = flow->run();

    // Measure execution time
    stopTimer(&timer);

    cerr << "Execution time: ";
    printTimer(cerr, &timer);
    cerr << endl;
    
    // Output the result of the flow
    flow->outputMappingAsXML(out);
    
    // Clear the output buffer
    out.flush();

    // Flow failed?
    if (status != SDF3Flow::FlowCompleted)
    {
        logError("Failed to complete flow.");
        return NULL;
    }

    // Output result of flow as HTML?
    if (outputAsHTML)
        flow->outputMappingAsHTML();
    
    // Retrieve system usage in XML format
    xmlSDF3Node = flow->createSDF3Node();
    
    // Done
    return CGetChildNode(xmlSDF3Node, "systemUsage");
}

/**
 * mapApplicationGraphsToArchitectureGraph ()
 * Map all application graphs in the order specified by the settings to
 * the architecture graph.
 */
bool mapApplicationGraphsToArchitectureGraph(ostream &out)
{
    CNode *xmlSystemUsage = settings.xmlSystemUsage;

    for (list<CNode*>::iterator appsIter = settings.xmlAppGraphs.begin();
            appsIter != settings.xmlAppGraphs.end(); appsIter++)
    {
        CNode *xmlAppGraph = *appsIter;
        CString appName;
        ofstream outFile;

        // Output every mapping to a different file
        if (!settings.outputFile.empty() && settings.xmlAppGraphs.size() != 1)
        {
            appName = CGetAttribute(xmlAppGraph, "name");
            outFile.open(appName + "_" + settings.outputFile);
            out.rdbuf(outFile.rdbuf());
        }
        
        // Map application to architecture
        xmlSystemUsage = mapApplicationGraphToArchitectureGraph(xmlAppGraph,
                                    settings.xmlArchGraph, xmlSystemUsage, out, 
                                    settings.outputAsHTML);
        
        // Flow failed?
        if (xmlSystemUsage == NULL)
            return false;
    }

    return true;
}

/**
 * main ()
 * It does none of the hard work, but it is very needed...
 */
int main(int argc, char **argv)
{
    int exit_status = 0;
    
    try
    {
        // Initialize the program
        if (!initSettings(argc, argv))
            return 1;

        // Run mapping flow
        if (mapApplicationGraphsToArchitectureGraph(settings.outputStream))
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
