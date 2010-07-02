/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   flow.h
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
 * $Id: flow.h,v 1.5 2008/03/20 16:16:18 sander Exp $
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

#ifndef SDF_FLOW_FLOW_H_INCLUDED
#define SDF_FLOW_FLOW_H_INCLUDED

#include "../tile_allocation/binding.h"
#include "../noc_allocation/mapping/nocmapping.h"

/**
 * Redefine names of Tile binding and NoC scheduling to get meaningfull 
 * names in the mapping flow.
 */
typedef Binding TileMapping;

/**
 * SDFG mapping to MP-SoC flow
 */
class SDF3Flow
{
public:
    // State of the mapping flow
    enum FlowState { FlowStart, 
                     FlowModelNonLocalMemory,
                     FlowComputeStorageDist,
                     FlowSelectStorageDist,
                     FlowEstimateStorageDist,
                     FlowEstimateLatencyConstraint,
                     FlowEstimateBandwidthConstraint,
                     FlowBindSDFGtoTile,
                     FlowStaticOrderScheduleTiles,
                     FlowAllocateTDMAtimeSlices,
                     FlowOptimizeStorageSpaceAllocations,
                     FlowExtractCommunicationConstraints,
                     FlowScheduleCommunication,
                     FlowUpdateBandwidthAllocations,
                     FlowCompleted,
                     FlowFailed };
    
public:
    // Constructor
    SDF3Flow(SDFflowType type, CNode *xmlAppGraph, CNode *xmlArchGraph, 
                CNode *xmlSystemUsage);
    
    // Destructor
    ~SDF3Flow();
    
    // Flow type
    SDFflowType getFlowType() const { return flowType; };
    
    // Execute design flow
    FlowState run();
    
    // Run flow step-by-step
    bool getStepMode() const { return stepMode; };
    void setStepMode(bool flag) { stepMode = flag; };
    
    // Settings for tile binding and scheduling phase
    void setTileMappingAlgo(TileMapping *a) { tileMapping = a; };
    
    // Settings for NoC routing and scheduling phase
    void setNoCMappingAlgo(NoCMapping *a) { nocMapping = a; };

    // Mapping results
    CNode *createSDF3Node();
    void outputMappingAsXML(ostream &out);
    void outputMappingAsHTML();
    
private:
    // Application graph
    TimedSDFgraph *getAppGraph() { return appGraph; };
    TimedSDFgraph *createAppGraph(CNode *xmlAppGraph);
    
    // Platform graph
    PlatformGraph *getPlatformGraph() { return platformGraph; };
    PlatformGraph *createPlatformGraph(CNode *xmlPlatformGraph);
    void setSystemUsagePlatformGraph(PlatformGraph *g, CNode *systemUsage);
    
    // State of the design flow
    void setNextStateOfFlow(FlowState s) { stateOfFlow = s; };
    FlowState getStateOfFlow() { return stateOfFlow; };

private:
    // Steps of the design flow
    void checkInputDesignFlow();
    void modelNonLocalMemoryAccesses();
    void computeStorageDistributions();
    void selectStorageDistribution();
    void estimateStorageConstraints();
    void estimateLatencyConstraints();
    void estimateBandwidthConstraints();
    void bindSDFGtoTiles();
    void constructStaticOrderScheduleTiles();
    void allocateTDMAtimeSlices();
    void optimizeStorageSpaceAllocations();
    void extractCommunicationConstraints();
    void scheduleCommunication();
    void updateBandwidthAllocations();

    // User interaction when running flow in step-by-step mode
    void handleUserInteraction();

    // Storage distributions in XML format
    CNode *createStorageDistributionsNode(
            StorageDistributionSet *distributions);
    
private:
    // Flow type
    SDFflowType flowType;
    
    // Run flow step-by-step
    bool stepMode;
    
    // XML description of the application and architecture
    CNode *xmlAppGraph;
    CNode *xmlArchGraph;
    CNode *xmlSystemUsage;

    // Application graph
    TimedSDFgraph *appGraph;
    
    // Platform graph
    PlatformGraph *platformGraph;
    
    // Tile binding and scheduling object
    TileMapping *tileMapping;
    
    // NoC routing and scheduling object
    NoCMapping *nocMapping;
    
    // State of the design flow
    FlowState stateOfFlow;

    // Storage-space / throughput exploration algorithm
    SDFstateSpaceBufferAnalysis bufferAnalysisAlgo;

    // Minimal storage distributions
    StorageDistributionSet *minStorageDistributions;
    StorageDistributionSet *selectedStorageDistributionSet;
    StorageDistribution *selectedStorageDistribution;
};

#endif
