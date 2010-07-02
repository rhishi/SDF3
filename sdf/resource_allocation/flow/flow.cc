/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   flow.cc
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
 * $Id: flow.cc,v 1.4 2008/03/20 16:16:18 sander Exp $
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

#include "flow.h"
#include "../mpsoc_arch/xml.h"
#include "../../output/xml/xml.h"
#include "../../output/html/html.h"

/**
 * SDF3Flow ()
 * Constructor.
 */
SDF3Flow::SDF3Flow(SDFflowType type, CNode *xmlAppGraph, CNode *xmlArchGraph,
        CNode *xmlSystemUsage)
    :
        flowType(type),
        xmlAppGraph(xmlAppGraph),
        xmlArchGraph(xmlArchGraph),
        xmlSystemUsage(xmlSystemUsage)
{
    // Run flow in one go
    stepMode = false;
    
    // Application graph
    appGraph = createAppGraph(xmlAppGraph);
    
    // Platform graph
    platformGraph = createPlatformGraph(xmlArchGraph);
    setSystemUsagePlatformGraph(platformGraph, xmlSystemUsage);
    
    // Tile binding and scheduling algorithm
    tileMapping = NULL;
    
    // NoC routing and scheduling algorithm
    nocMapping = NULL;
    
    // Design flow is in its initial state
    stateOfFlow = FlowStart;
    
    // No storage space distributions found
    minStorageDistributions = NULL;
    selectedStorageDistributionSet = NULL;
    selectedStorageDistribution = NULL;
}
    
/**
 * ~SDF3Flow ()
 * Destructor.
 */
SDF3Flow::~SDF3Flow()
{
    delete tileMapping;
    delete nocMapping;
    delete appGraph;
    delete platformGraph;
}

/**
 * createAppGraph ()
 * The function creates an timed SDFG according to the XML specification.
 */
TimedSDFgraph *SDF3Flow::createAppGraph(CNode *xmlAppGraph)
{
    CNode *sdfNode, *sdfPropertiesNode;
    TimedSDFgraph *g;

    // Select nodes in the XML structure
    sdfNode = CGetChildNode(xmlAppGraph, "sdf");
    sdfPropertiesNode = CGetChildNode(xmlAppGraph, "sdfProperties");
    
    // Construct the graph
    g = new TimedSDFgraph();
    g->construct(sdfNode, sdfPropertiesNode);

    return g;
}

/**
 * createPlatformGraph ()
 * The function creates a platform graph according to the XML specification.
 */
PlatformGraph *SDF3Flow::createPlatformGraph(CNode *xmlPlatformGraph)
{
    return constructPlatformGraph(xmlPlatformGraph);
}

/**
 * setSystemUsagePlatformGraph ()
 * The function sets the usage of the platform graph as specified in the 
 * XML specs.
 */
void SDF3Flow::setSystemUsagePlatformGraph(PlatformGraph *g, CNode *systemUsage)
{
    setUsagePlatformGraph(g, systemUsage);    
}

/**
 * checkInputDesignFlow ()
 * The function checks that all required inputs are present and it initializes
 * the used data-structures.
 */
void SDF3Flow::checkInputDesignFlow()
{
    // Output current state of the flow
    logInfo("Check input design flow.");
    
    if (tileMapping == NULL)
        throw CException("[ERROR] No tile mapping algorithm specified.");
        
    if (nocMapping == NULL)
        throw CException("[ERROR] No NoC mapping algorithm specified.");

    // Link application and platform to tile mapping algorithm
    tileMapping->setAppGraph(getAppGraph());
    tileMapping->setArchGraph(getPlatformGraph());
    
    // Link application, platform, interconnect and usage to NoC mapping algo
    nocMapping->init(getAppGraph(), getPlatformGraph(), xmlArchGraph,
                        xmlSystemUsage);

    // Advance to next state in the flow
    setNextStateOfFlow(FlowModelNonLocalMemory);    
}
   
/**
 * run ()
 * Execute the design flow. The function returns the last state reached by 
 * the design flow. Its value is equal to 'FlowCompleted' when all phases
 * have been executed succesfully or else it is 'FlowFailed' which indicates
 * that a step of the flow could not be completed succesfully.
 */
SDF3Flow::FlowState SDF3Flow::run()
{
    CTimer timer;
    
    do
    {
        // Measure execution time
        startTimer(&timer);

        switch (getStateOfFlow())
        {
            case FlowStart:
                checkInputDesignFlow();
                break;
                        
            case FlowModelNonLocalMemory:
                modelNonLocalMemoryAccesses();
                break;
                
            case FlowComputeStorageDist:
                computeStorageDistributions();
                break;
                
            case FlowSelectStorageDist:
                selectStorageDistribution();
                break;
                
            case FlowEstimateStorageDist:
                estimateStorageConstraints();
                break;

            case FlowEstimateLatencyConstraint:
                estimateLatencyConstraints();
                break;

            case FlowEstimateBandwidthConstraint:
                estimateBandwidthConstraints();
                break;
                
            case FlowBindSDFGtoTile:
                bindSDFGtoTiles();
                break;
                
            case FlowStaticOrderScheduleTiles:
                constructStaticOrderScheduleTiles();
                break;
                
            case FlowAllocateTDMAtimeSlices:
                allocateTDMAtimeSlices();
                break;
                
            case FlowOptimizeStorageSpaceAllocations:
                optimizeStorageSpaceAllocations();
                break;
                
            case FlowExtractCommunicationConstraints:
                extractCommunicationConstraints();
                break;
                
            case FlowScheduleCommunication:
                scheduleCommunication();
                break;
                
            case FlowUpdateBandwidthAllocations:
                updateBandwidthAllocations();
                break;
            
            default:
                break;
        }

        // Measure execution time
        stopTimer(&timer);

        cerr << "[INFO] Step took: ";
        printTimer(cerr, &timer);
        cerr << endl;
        
        if (getStepMode())
        {
            handleUserInteraction();
        }
    
    } while (getStateOfFlow() != FlowCompleted
                && getStateOfFlow() != FlowFailed);
    
    return getStateOfFlow();
}

/**
 * handleUserInteraction ()
 * The function request the user for the next action to perform. Possible
 * actions include continuing with the next step of the flow, completing the
 * remaining flow or printing the current result of the flow to the terminal.
 */
void SDF3Flow::handleUserInteraction()
{
    bool done = false;
    char cmd, c;
    
    do
    {
        // Print message
        cout << "Command: (n)ext step / (c)ontinue flow / print (s)tate / print (h)tml? ";

        // First character determines command
        cmd = cin.get();
        
        // Command is end-of-line (default action: step)
        if (cmd == '\n')
            cmd = 'n';
        else
        {
            // Remove remaining characters on current line from the input
            do { c = cin.get(); } while (c != '\n');
        }

        switch (cmd)
        {
            case 'n':
                done = true;
                break;

            case 'c':
                setStepMode(false);
                done = true;
                break;

            case 's':
                outputMappingAsXML(cout);
                break;

            case 'h':
                outputMappingAsHTML();
                break;

            default:
                break;
        }
    } while (!done);
}

/**
 * createStorageDistributionsNode ()
 * The function returns an XML node that describes the storage-space throughput
 * trade-off points.
 */
CNode *SDF3Flow::createStorageDistributionsNode(
        StorageDistributionSet *distributions)
{
    CNode *storageNode, *distributionsNode, *dNode, *chNode;
    StorageDistributionSet *set = distributions;
    StorageDistribution *d = NULL;

    // Create new node for trade-off space    
    storageNode = CNewNode("storageThroughputTradeOffs");
    
    while (set != NULL)
    {
        // Create node for all distributions with same size
        distributionsNode = CAddNode(storageNode, "distributionsSet");
        CAddAttribute(distributionsNode, "sz", set->sz);
        CAddAttribute(distributionsNode, "thr", CString(set->thr));
        
        // Select first distribution in set
        d = set->distributions;
        
        while (d != NULL)
        {
            // Create node for specific distribution
            dNode = CAddNode(distributionsNode, "distribution");
            
            for (uint i = 0; i < appGraph->nrChannels(); i++)
            {
                chNode = CAddNode(dNode, "ch");
                CAddAttribute(chNode, "name",
                                        appGraph->getChannel(i)->getName());
                CAddAttribute(chNode, "sz", d->sp[i]);
            }
            
            // Next distribution
            d = d->next;
        }
        
        // Next distribution set
        set = set->next;
    }
    
    return storageNode;
}

/**
 * createSDF3Node ()
 * The function returns an sdf3 node that describes the current status of
 * the mapping flow.
 */
CNode *SDF3Flow::createSDF3Node()
{
    CNode *sdf3Node, *archGraphNode, *mappingNode, *systemUsageNode;
    
    // SDF mapping node
    sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                            "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                            "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");

    // Application graph node
    CAddNode(sdf3Node, createApplicationGraphNode(appGraph));
        
    // Architecture graph node
    archGraphNode = CAddNode(sdf3Node,
                                createPlatformGraphNode(platformGraph));
    if (nocMapping != NULL)
        CAddNode(archGraphNode, nocMapping->createNetworkNode());
    
    // Mapping node (to platform graph and to interconnect)
    mappingNode = CAddNode(sdf3Node, 
                                createMappingNode(platformGraph, appGraph));
    if (nocMapping != NULL)
        CAddNode(mappingNode, nocMapping->createMappingNode());
    
    // System usage node
    systemUsageNode = CAddNode(sdf3Node, createSystemUsageNode(platformGraph));
    if (nocMapping != NULL)
        nocMapping->addUsageNode(systemUsageNode);

    // Storage-space / throughput trade-off points
    CAddNode(sdf3Node,
                    createStorageDistributionsNode(minStorageDistributions));
    
    // Messages node
    if (nocMapping != NULL)
        CAddNode(sdf3Node, nocMapping->getMessages());
    
    return sdf3Node;
}

/**
 * outputMappingAsXML ()
 * The function ouputs an sdf3 node to the supplied stream.
 */
void SDF3Flow::outputMappingAsXML(ostream &out)
{
    CNode *sdf3Node;
    CDoc *doc;
    
    // SDF mapping node
    sdf3Node = createSDF3Node();
    
    // Create document and save it
    doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);
}

/**
 * outputMappingAsHTML ()
 * The function ouputs the mapping to a set of HTML pages.
 */
void SDF3Flow::outputMappingAsHTML()
{
    SDFconvertToHTML sdfConvertToHTML;
    
    // Associate the graphs with the converter
    sdfConvertToHTML.setSDFgraph(getAppGraph());
    sdfConvertToHTML.setPlatformGraph(getPlatformGraph());
    sdfConvertToHTML.setSetOfNoCScheduleProblems(
                                nocMapping->getSetOfNoCScheduleProblems());
    
    // Convert...
    sdfConvertToHTML.convert(getAppGraph()->getName() + "_");
}

