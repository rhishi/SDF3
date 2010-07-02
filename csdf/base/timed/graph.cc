/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   timed_graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 19, 2005
 *
 *  Function        :   Timed CSDF graph
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: graph.cc,v 1.3 2008/03/22 14:24:21 sander Exp $
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

#include "graph.h"

/**
 * TimedCSDFgraph ()
 * Constructor.
 */
TimedCSDFgraph::TimedCSDFgraph(CSDFcomponent &c)
    :
        CSDFgraph(c)
{
}

/**
 * ~TimedCSDFgraph ()
 * Destructor.
 */
TimedCSDFgraph::~TimedCSDFgraph()
{
}

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 */
TimedCSDFgraph *TimedCSDFgraph::create(CSDFcomponent &c) const
{
    return new TimedCSDFgraph(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * The properties of this graph are also assigned to the new graph.
 */
TimedCSDFgraph *TimedCSDFgraph::clone(CSDFcomponent &c) const
{
    TimedCSDFgraph *g = createCopy(c);

    // Actors
    for (CSDFactorsCIter iter = actorsBegin(); iter != actorsEnd(); iter++)
    {
        CSDFcomponent component = CSDFcomponent(g, g->nrActors());
        CSDFactor *a = (*iter)->clone(component);
        g->addActor(a);
    }
    
    // Channels
    for (CSDFchannelsCIter iter = channelsBegin(); 
            iter != channelsEnd(); iter++)
    {
        CSDFcomponent component = CSDFcomponent(g, g->nrChannels());
        CSDFchannel *ch = (*iter)->clone(component);
        g->addChannel(ch);
    }
    
    return g;
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * The properties of this graph are also assigned to the new graph.
 */
TimedCSDFgraph *TimedCSDFgraph::clone() const
{
    CSDFcomponent comp = CSDFcomponent(getParent(), getId(), getName());
    return clone(comp);
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * The properties of this graph are also assigned to the new graph.
 */
TimedCSDFgraph *TimedCSDFgraph::createCopy(CSDFcomponent &c) const
{
    TimedCSDFgraph *g = new TimedCSDFgraph(c);
    
    // Properties
    g->setName(getName());
    g->setType(getType());
    
    return g;
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
TimedCSDFactor *TimedCSDFgraph::createActor()
{
    CSDFcomponent c = CSDFcomponent(this, nrActors());
    TimedCSDFactor *a = new TimedCSDFactor(c);
    a->setName(CString("_a") + CString(nrActors()+1));
    addActor(a);
    
    return a;
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
TimedCSDFactor *TimedCSDFgraph::createActor(CSDFcomponent &c)
{
    TimedCSDFactor *a = new TimedCSDFactor(c);
    addActor(a);
    
    return a;
}

/**
 * createChannel ()
 * Create a new channel on the graph.
 */
TimedCSDFchannel *TimedCSDFgraph::createChannel(CSDFcomponent &c)
{
    TimedCSDFchannel *ch = new TimedCSDFchannel(c);
    addChannel(ch);
    
    return ch;
}

/**
 * createChannel ()
 * The function creates a channel between the source and destination actor.
 * Ports with the supplied rates are added to these actors.
 */
TimedCSDFchannel *TimedCSDFgraph::createChannel(CSDFactor *src, 
        CSDFrate rateSrc, CSDFactor *dst, CSDFrate rateDst)
{
    // Create new channel
    CSDFcomponent c = CSDFcomponent(this, nrChannels());
    TimedCSDFchannel *ch = createChannel(c);
    ch->setName(CString("_ch") + CString(nrChannels()));
    
    // Create ports on the actors
    CSDFport *srcP = src->createPort("out", rateSrc);
    srcP->setName(CString("_p") + CString(src->nrPorts()));
    CSDFport *dstP = dst->createPort("in", rateDst);
    dstP->setName(CString("_p") + CString(dst->nrPorts()));
    
    // Connect channel
    ch->connectSrc(srcP);
    ch->connectDst(dstP);
    
    return ch;
}

/**
 * print ()
 * Print the graph to the supplied output stream.
 */
ostream &TimedCSDFgraph::print(ostream &out)
{
    out << "Graph (" << getName() << ")" << endl;
    out << "id:        " << getId() << endl;
    out << "type:      " << getType() << endl;
    out << endl;

    for (CSDFactorsIter iter = actorsBegin(); iter != actorsEnd(); iter++)
    {
        CSDFactor *a = *iter;
        
        ((TimedCSDFactor*)(a))->print(out);
    }

    for (CSDFchannelsIter iter = channelsBegin();
            iter != channelsEnd(); iter++)
    {
        CSDFchannel *c = *iter;
        
        ((TimedCSDFchannel*)(c))->print(out);
    }
    
    out << endl;
    
    return out;
}

/**
 * actorProperties ()
 * Set the properties of a timed actor.
 */
void actorProperties(TimedCSDFactor *a, const CNodePtr propertiesNode)
{
    TimedCSDFactor::Processor *p;
    CStrings execTimes;
    
    // Processor definitions
    for (CNode *procNode = CGetChildNode(propertiesNode, "processor"); 
            procNode != NULL; procNode = CNextNode(procNode, "processor"))
    {
        if (!CHasAttribute(procNode, "type"))
            throw CException("Processor must have a type");
    
        // Add a processor to the actor
        p = a->addProcessor(CGetAttribute(procNode, "type"));
        
        // Execution time
        if (CHasChildNode(procNode, "executionTime"))
        {
            CNode *execTimeNode = CGetChildNode(procNode, "executionTime");

            if (!CHasAttribute(execTimeNode, "time"))
                throw CException("Execution time not specified");

            p->execTime = CSDFtimeSequence(CGetAttribute(execTimeNode, "time"));

            // Is the length of the execution time sequence equal to the 
            // sequenceLength of the actor?
            if (p->execTime.size() != a->sequenceLength())
                throw CException("[ERROR] length of execution time sequences "
                                 "not equal.");
        }

        // Memory
        if (CHasChildNode(procNode, "memory"))
        {
            CNode *memoryNode;
            memoryNode = CGetChildNode(procNode, "memory");

            if (CHasChildNode(memoryNode, "stateSize"))
            {
                CNode *stateSizeNode;
                stateSizeNode = CGetChildNode(memoryNode, "stateSize");

                if (!CHasAttribute(stateSizeNode, "max"))
                    throw CException("No maximum state size given.");

                int sz = CGetAttribute(stateSizeNode, "max");
                p->stateSize = sz;
            }
        }
        
        // Default processor?
        if (CHasAttribute(procNode, "default"))
        {
            a->setDefaultProcessor(p->type);
        }
    }
}

/**
 * channelProperties ()
 * Set the properties of a timed channel.
 */
void channelProperties(TimedCSDFchannel *c, const CNodePtr propertiesNode)
{
    // Buffer size
    if (CHasChildNode(propertiesNode, "bufferSize"))
    {
        CNode *bufferSizeNode = CGetChildNode(propertiesNode, "bufferSize");
        CSDFbufferSize bufferSize;

        bufferSize.sz = -1;
        bufferSize.src = -1;
        bufferSize.dst = -1;
        bufferSize.mem = -1;
        
        if (CHasAttribute(bufferSizeNode, "sz"))
            bufferSize.sz = CGetAttribute(bufferSizeNode, "sz");
            
        if (CHasAttribute(bufferSizeNode, "src"))
            bufferSize.src = CGetAttribute(bufferSizeNode, "src");

        if (CHasAttribute(bufferSizeNode, "dst"))
            bufferSize.dst = CGetAttribute(bufferSizeNode, "dst");

        if (CHasAttribute(bufferSizeNode, "mem"))
            bufferSize.mem = CGetAttribute(bufferSizeNode, "mem");
        
        c->setBufferSize(bufferSize);
    }

    // Token size
    if (CHasChildNode(propertiesNode, "tokenSize"))
    {
        CNode *tokenSizeNode = CGetChildNode(propertiesNode, "tokenSize");
        
        if (CHasAttribute(tokenSizeNode, "sz"))
        {
            int sz = CGetAttribute(tokenSizeNode, "sz");
            c->setTokenSize(sz);
        }
    }

    // Token type
    if (CHasChildNode(propertiesNode, "tokenType"))
    {
        CNode *tokenTypeNode = CGetChildNode(propertiesNode, "tokenType");
        
        if (CHasAttribute(tokenTypeNode, "type"))
        {
            CString type = CGetAttribute(tokenTypeNode, "type");
            c->setTokenType(type);
        }
    }

    // Bandwidth
    if (CHasChildNode(propertiesNode, "bandwidth"))
    {
        CNode *bwNode = CGetChildNode(propertiesNode, "bandwidth");
        
        if (CHasAttribute(bwNode, "min"))
        {
            double sz = CGetAttribute(bwNode, "min");
            c->setMinBandwidth(sz);
        }
    }

    // Latency
    if (CHasChildNode(propertiesNode, "latency"))
    {
        CNode *latencyNode = CGetChildNode(propertiesNode, "latency");
        
        if (CHasAttribute(latencyNode, "min"))
        {
            int sz = CGetAttribute(latencyNode, "min");
            c->setMinLatency(sz);
        }
    }
}

/**
 * graphProperties ()
 * Set the properties of a timed graph.
 */
void graphProperties(TimedCSDFgraph *g, const CNodePtr propertiesNode)
{
    // Timing constraints
    for (CNode *timeNode = CGetChildNode(propertiesNode, "timeConstraints");
            timeNode != NULL; timeNode = CNextNode(timeNode, "timeConstraints"))
    {
        if (CHasChildNode(timeNode, "throughput"))
        {
            CNode *throughputNode;
            throughputNode = CGetChildNode(timeNode, "throughput");
            
            CString throughput = CGetNodeContent(throughputNode);

            g->setThroughputConstraint((double)(throughput));
        }        
    }
}

/**
 * constructTimedSDFgraphStructure ()
 * Construct a timed SDF graph.
 */
TimedCSDFgraph *constructTimedCSDFgraphStructure(const CNodePtr csdfNode)
{
    CId actorId = 0, channelId = 0, portId;
    TimedCSDFgraph *g;
    
    // Construct CSDF graph
    CSDFcomponent component = CSDFcomponent(NULL, 0);
    g = new TimedCSDFgraph(component);
    g->construct(csdfNode);
    
    // Construct all actors
    for (CNodePtr actorNode = CGetChildNode(csdfNode, "actor");
                actorNode != NULL; actorNode = CNextNode(actorNode, "actor"))
    {
        // Construct actor
        component = CSDFcomponent(g, actorId);
        TimedCSDFactor *a = new TimedCSDFactor(component);
        a->construct(actorNode);

        // Construct all ports
        portId = 0;
        for (CNodePtr portNode = CGetChildNode(actorNode, "port");
                    portNode != NULL; portNode = CNextNode(portNode, "port"))
        {
            // Construct port
            component = CSDFcomponent(a, portId);
            CSDFport *p = new CSDFport(component);
            p->construct(portNode);
            
            // Add port to actor
            a->addPort(p);
            
            // Next port
            portId++;
        }    

        // Add actor to graph
        g->addActor(a);
        
        // Next actor
        actorId++;
    }

    // Construct all channels
    for (CNodePtr channelNode = CGetChildNode(csdfNode, "channel");  
            channelNode != NULL; 
                channelNode = CNextNode(channelNode, "channel"))
    {
        // Construct channel
        component = CSDFcomponent(g, channelId);
        TimedCSDFchannel *c = new TimedCSDFchannel(component);
        c->construct(channelNode);
 
        // Add channel to graph
        g->addChannel(c);
       
        // Next channel
        channelId++;
    }

    return g;
}

/**
 * constructTimedSDFgraph ()
 * Construct a timed SDF graph.
 */
TimedCSDFgraph *constructTimedCSDFgraph(const CNodePtr csdfNode,
        const CNodePtr csdfPropertiesNode)
{
    TimedCSDFgraph *g;
    
    // Create graph structure
    g = constructTimedCSDFgraphStructure(csdfNode);

    // Any properties to be set?
    if (csdfPropertiesNode == NULL)
        return g;
    
    // Set all properties
    for (CNode *propertiesNode = CGetChildNode(csdfPropertiesNode);
            propertiesNode != NULL; propertiesNode = CNextNode(propertiesNode))
    {
        if (CIsNode(propertiesNode, "actorProperties"))
        {
            if (!CHasAttribute(propertiesNode, "actor"))
                throw CException("Missing 'actor' in 'actorProperties'");
                
            TimedCSDFactor *a;
            a = (TimedCSDFactor*)g->getActor(CGetAttribute(propertiesNode,
                                                "actor"));
            
            actorProperties(a, propertiesNode);
        }
        else if (CIsNode(propertiesNode, "channelProperties"))
        {
            if (!CHasAttribute(propertiesNode, "channel"))
                throw CException("Missing 'channel' in 'channelProperties'");
                
            TimedCSDFchannel *c;
            c = (TimedCSDFchannel*)g->getChannel(CGetAttribute(propertiesNode,
                                                "channel"));
            
            channelProperties(c, propertiesNode);
        }
        else if (CIsNode(propertiesNode, "graphProperties"))
        {
            graphProperties(g, propertiesNode);
        }
    }
    
    return g;
}
