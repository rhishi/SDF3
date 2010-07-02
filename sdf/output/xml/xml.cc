/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 29, 2005
 *
 *  Function        :   Output SDF graph in XML format
 *
 *  History         :
 *      29-07-05    :   Initial version.
 *
 * $Id: xml.cc,v 1.2 2008/03/17 14:07:37 sander Exp $
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

#include "xml.h"

CNode *createSDFnode(SDFgraph *g)
{
    CNode *sdfNode;
    
    // SDF node
    sdfNode = CNewNode("sdf");
    CAddAttribute(sdfNode, "name", g->getName());
    CAddAttribute(sdfNode, "type", g->getType());
    
    // Actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        CNode *actorNode;
        
        // Actor node
        actorNode = CAddNode(sdfNode, "actor");
        CAddAttribute(actorNode, "name", a->getName());
        CAddAttribute(actorNode, "type", a->getType());
        
        // Ports
        for (SDFportsIter iterP = a->portsBegin();
                iterP != a->portsEnd(); iterP++)
        {
            SDFport *p = *iterP;
            CNode *portNode;
            
            // Port node
            portNode = CAddNode(actorNode, "port");
            CAddAttribute(portNode, "name", p->getName());
            CAddAttribute(portNode, "type", p->getTypeAsString());
            CAddAttribute(portNode, "rate", p->getRate());
        }
    }
    
    // Channels
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        CNode *channelNode;

        // Channel node
        channelNode = CAddNode(sdfNode, "channel");
        CAddAttribute(channelNode, "name", c->getName());
        CAddAttribute(channelNode, "srcActor", c->getSrcActor()->getName());
        CAddAttribute(channelNode, "srcPort", c->getSrcPort()->getName());
        CAddAttribute(channelNode, "dstActor", c->getDstActor()->getName());
        CAddAttribute(channelNode, "dstPort", c->getDstPort()->getName());
        if (c->getInitialTokens() != 0)
            CAddAttribute(channelNode, "initialTokens", c->getInitialTokens());
    }
    
    return sdfNode;
}

CNode *createSDFpropertiesNode(TimedSDFgraph *g)
{
    CNode *sdfPropNode;
    
    // SDF properties node
    sdfPropNode = CNewNode("sdfProperties");

    // Actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        TimedSDFactor *a = (TimedSDFactor*)(*iter);
        CNode *actorNode;
        
        // Actor node
        actorNode = CAddNode(sdfPropNode, "actorProperties");
        CAddAttribute(actorNode, "actor", a->getName());

        // Processors
        for (TimedSDFactor::ProcessorsIter iterP = a->processorsBegin();
                iterP != a->processorsEnd(); iterP++)
        {
            TimedSDFactor::Processor *p = *iterP;
            CNode *procNode, *execTimeNode;
            
            // Processor node
            procNode = CAddNode(actorNode, "processor");
            CAddAttribute(procNode, "type", p->type);            
            
            // Default processor?
            if (a->getDefaultProcessor() == p->type)
                CAddAttribute(procNode, "default", "true");            

            // Execution time
            if (p->execTime != SDFTIME_MAX)
            {
                execTimeNode = CAddNode(procNode, "executionTime");
                CAddAttribute(execTimeNode, "time", p->execTime);
            }

            // State size
            if (p->stateSize != CSIZE_MAX)
            {
                CNode *memoryNode, *stateSizeNode;

                memoryNode = CAddNode(procNode, "memory");
                stateSizeNode = CAddNode(memoryNode, "stateSize");
                CAddAttribute(stateSizeNode, "max", p->stateSize);
            }
        }
    }
    
    // Channels
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        TimedSDFchannel *c = (TimedSDFchannel*)(*iter);
        CNode *channelNode;
        TimedSDFchannel::BufferSize bufferSize;
        
        // Channel node
        channelNode = CAddNode(sdfPropNode, "channelProperties");
        CAddAttribute(channelNode, "channel", c->getName());
        
        // Buffer size
        bufferSize = c->getBufferSize();
        if (bufferSize.sz != -1)
        {
            CNode *bufferSzNode = CAddNode(channelNode, "bufferSize");
            CAddAttribute(bufferSzNode, "sz", bufferSize.sz);
            
            if (bufferSize.src != -1)
                CAddAttribute(bufferSzNode, "src", bufferSize.src);
                        
            if (bufferSize.dst != -1)
                CAddAttribute(bufferSzNode, "dst", bufferSize.dst);
                        
            if (bufferSize.mem != -1)
                CAddAttribute(bufferSzNode, "mem", bufferSize.mem);
        }
        
        // Token size
        if (c->getTokenSize() != -1)
        {
            CNode *tokenSzNode = CAddNode(channelNode, "tokenSize");
            CAddAttribute(tokenSzNode, "sz", c->getTokenSize());
        }

        // Token type
        if (!c->getTokenType().empty())
        {
            CNode *tokenTypeNode = CAddNode(channelNode, "tokenType");
            CAddAttribute(tokenTypeNode, "type", c->getTokenType());
        }
        
        // Bandwidth
        if (c->getMinBandwidth() != 0)
        {
            CNode *bwNode = CAddNode(channelNode, "bandwidth");
            CAddAttribute(bwNode, "min", CString(c->getMinBandwidth()));
        }

        // Latency
        if (c->getMinLatency() != 0)
        {
            CNode *latencyNode = CAddNode(channelNode, "latency");
            CAddAttribute(latencyNode, "min", c->getMinLatency());
        }
    }
        
    // Graph
    CNode *graphPropNode;
    graphPropNode = CAddNode(sdfPropNode, "graphProperties");
    
    // Throughput constraint
    if (g->getThroughputConstraint() != 0)
    {
        CNode *timeConstraintsNode, *throughputNode;
        CString thr = CString(g->getThroughputConstraint().value());
        timeConstraintsNode = CAddNode(graphPropNode, "timeConstraints");
        throughputNode = CAddNode(timeConstraintsNode, "throughput", thr);
    }
    
    return sdfPropNode;
}

/**
 * createApplicationGraphNode ()
 * The function returns an SDF graph in XML format.
 */
CNode *createApplicationGraphNode(TimedSDFgraph *g)
{
    // Application graph node
    CNode *appNode = CNewNode("applicationGraph");
   
    // SDF node
    CAddNode(appNode, createSDFnode(g));

    // SDF properties node
    CAddNode(appNode, createSDFpropertiesNode(g));

    return appNode;
}

/**
 * outputSDFasXML ()
 * The function outputs a SDF graph in XML format.
 */
void outputSDFasXML(SDFgraph *g, ostream &out)
{
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                            "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                            "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");
    
    // Application graph node
    CNode *appNode = CAddNode(sdf3Node, "applicationGraph");
   
    // SDF node
    CAddNode(appNode, createSDFnode(g));
    
    // Create document and save it
    CDoc *doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);
}

/**
 * outputSDFasXML ()
 * The function outputs a SDF graph in XML format.
 */
void outputSDFasXML(TimedSDFgraph *g, ostream &out)
{
    // SDF mapping node
    CNode *sdf3Node = CNewNode("sdf3");
    CAddAttribute(sdf3Node, "version", "1.0");
    CAddAttribute(sdf3Node, "type", "sdf");
    CAddAttribute(sdf3Node, "xmlns:xsi",
                            "http://www.w3.org/2001/XMLSchema-instance");
    CAddAttribute(sdf3Node, "xsi:noNamespaceSchemaLocation",
                            "http://www.es.ele.tue.nl/sdf3/xsd/sdf3-sdf.xsd");
    
    // Application graph node
    CNode *appNode = CAddNode(sdf3Node, "applicationGraph");
   
    // SDF node
    CAddNode(appNode, createSDFnode(g));

    // SDF properties node
    CAddNode(appNode, createSDFpropertiesNode(g));
    
    // Create document and save it
    CDoc *doc = CNewDoc(sdf3Node);
    CSaveFile(out, doc);
}
