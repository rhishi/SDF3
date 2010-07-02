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
 *  Function        :   Timed SDF graph
 *
 *  History         :
 *      19-07-05    :   Initial version.
 *
 * $Id: graph.cc,v 1.2 2008/03/17 14:07:37 sander Exp $
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
 * TimedSDFgraph ()
 * Constructor.
 */
TimedSDFgraph::TimedSDFgraph(SDFcomponent &c)
    :
        SDFgraph(c)
{
}

/**
 * TimedSDFgraph ()
 * Constructor.
 */
TimedSDFgraph::TimedSDFgraph()
    :
        SDFgraph()
{
}

/**
 * ~TimedSDFgraph ()
 * Destructor.
 */
TimedSDFgraph::~TimedSDFgraph()
{
}

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF graph object.
 */
TimedSDFgraph *TimedSDFgraph::create(SDFcomponent &c) const
{
    return new TimedSDFgraph(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF graph object.
 * The properties of this graph are also assigned to the new graph.
 */
TimedSDFgraph *TimedSDFgraph::clone(SDFcomponent &c) const
{
    TimedSDFgraph *g = createCopy(c);

    // Actors
    for (SDFactorsCIter iter = actorsBegin(); iter != actorsEnd(); iter++)
    {
        SDFcomponent component = SDFcomponent(g, g->nrActors());
        SDFactor *a = (*iter)->clone(component);
        g->addActor(a);
    }
    
    // Channels
    for (SDFchannelsCIter iter = channelsBegin(); 
            iter != channelsEnd(); iter++)
    {
        SDFcomponent component = SDFcomponent(g, g->nrChannels());
        SDFchannel *ch = (*iter)->clone(component);
        g->addChannel(ch);
    }
    
    return g;
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF graph object.
 * The properties of this graph are also assigned to the new graph.
 */
TimedSDFgraph *TimedSDFgraph::clone() const
{
    SDFcomponent comp = SDFcomponent(getParent(), getId(), getName());
    return clone(comp);
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF graph object.
 * The properties of this graph are also assigned to the new graph.
 */
TimedSDFgraph *TimedSDFgraph::createCopy(SDFcomponent &c) const
{
    TimedSDFgraph *g = new TimedSDFgraph(c);
    
    // Properties
    g->setName(getName());
    g->setType(getType());
    
    return g;
}

/**
 * construct ()
 * Set the properties of a timed graph.
 */
void TimedSDFgraph::construct(const CNodePtr sdfNode,
        const CNodePtr sdfPropertiesNode)
{
    // Construct SDFG
    SDFgraph::construct(sdfNode);
    
    // Any properties to be set?
    if (sdfPropertiesNode == NULL)
        return;
 
    // Set all properties
    for (CNode *propertiesNode = CGetChildNode(sdfPropertiesNode);
            propertiesNode != NULL; propertiesNode = CNextNode(propertiesNode))
    {
        if (CIsNode(propertiesNode, "actorProperties"))
        {
            if (!CHasAttribute(propertiesNode, "actor"))
                throw CException("Missing 'actor' in 'actorProperties'");
                
            TimedSDFactor *a;
            a = (TimedSDFactor*)getActor(CGetAttribute(propertiesNode,"actor"));
            
            a->setProperties(propertiesNode);
        }
        else if (CIsNode(propertiesNode, "channelProperties"))
        {
            if (!CHasAttribute(propertiesNode, "channel"))
                throw CException("Missing 'channel' in 'channelProperties'");
                
            TimedSDFchannel *c;
            c = (TimedSDFchannel*)getChannel(CGetAttribute(propertiesNode,
                                                            "channel"));
            
            c->setProperties(propertiesNode);
        }
        else if (CIsNode(propertiesNode, "graphProperties"))
        {
            setProperties(propertiesNode);
        }
    }
}

/**
 * setProperties ()
 * Set the properties of a timed actor.
 */
void TimedSDFgraph::setProperties(const CNodePtr propertiesNode)
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

            setThroughputConstraint((double)(throughput));
        }        
    }
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
TimedSDFactor *TimedSDFgraph::createActor()
{
    SDFcomponent c = SDFcomponent(this, nrActors());
    TimedSDFactor *a = new TimedSDFactor(c);
    a->setName(CString("_a") + CString(nrActors()+1));
    addActor(a);
    
    return a;
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
TimedSDFactor *TimedSDFgraph::createActor(SDFcomponent &c)
{
    TimedSDFactor *a = new TimedSDFactor(c);
    addActor(a);
 
    return a;
}

/**
 * createChannel ()
 * Create a new channel on the graph.
 */
TimedSDFchannel *TimedSDFgraph::createChannel(SDFcomponent &c)
{
    TimedSDFchannel *ch = new TimedSDFchannel(c);
    addChannel(ch);
    
    return ch;
}

/**
 * createChannel ()
 * The function creates a channel between the source and destination actor.
 * Ports with the supplied rates are added to these actors.
 */
TimedSDFchannel *TimedSDFgraph::createChannel(SDFactor *src, SDFrate rateSrc, 
        SDFactor *dst, SDFrate rateDst, uint initialTokens)
{
    // Create new channel
    SDFcomponent c = SDFcomponent(this, nrChannels());
    TimedSDFchannel *ch = createChannel(c);
    ch->setName(CString("_ch") + CString(nrChannels()));
    
    // Create ports on the actors
    SDFport *srcP = src->createPort(SDFport::Out, rateSrc);
    srcP->setName(CString("_p") + CString(src->nrPorts()));
    SDFport *dstP = dst->createPort(SDFport::In, rateDst);
    dstP->setName(CString("_p") + CString(dst->nrPorts()));
    
    // Connect channel
    ch->connectSrc(srcP);
    ch->connectDst(dstP);
    
    // Initial tokens
    ch->setInitialTokens(initialTokens);

    return ch;
}

/**
 * print ()
 * Print the graph to the supplied output stream.
 */
ostream &TimedSDFgraph::print(ostream &out)
{
    out << "Graph (" << getName() << ")" << endl;
    out << "id:        " << getId() << endl;
    out << "type:      " << getType() << endl;
    out << endl;

    for (SDFactorsIter iter = actorsBegin(); iter != actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        
        ((TimedSDFactor*)(a))->print(out);
    }

    for (SDFchannelsIter iter = channelsBegin();
            iter != channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        
        ((TimedSDFchannel*)(c))->print(out);
    }
    
    out << endl;
    
    return out;
}
