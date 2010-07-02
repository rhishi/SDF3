/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   CSDF graph
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: graph.cc,v 1.4 2008/09/18 07:38:21 sander Exp $
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
 * CSDFgraph ()
 * Constructor.
 */
CSDFgraph::CSDFgraph(CSDFcomponent &c)
    :
        CSDFcomponent(c)
{
}

/**
 * ~CSDFgraph ()
 * Destructor.
 */
CSDFgraph::~CSDFgraph()
{
    // Actors
    for (CSDFactorsIter iter = actors.begin(); iter != actors.end(); iter++)
    {
        CSDFactor *a = *iter;
        
        delete a;
    }
    
    // Channels
    for (CSDFchannelsIter iter = channels.begin();
            iter != channels.begin(); iter++)
    {
        CSDFchannel *c = *iter;
        
        delete c;
    }
}

/**
 * create ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 */
CSDFgraph *CSDFgraph::create(CSDFcomponent &c) const
{
    return new CSDFgraph(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * All properties, actors and channels of the graph are cloned.
 */
CSDFgraph *CSDFgraph::clone(CSDFcomponent &c) const
{
    CSDFgraph *g = new CSDFgraph(c);
    
    // Properties
    g->setName(getName());
    g->setType(getType());
    
    // Actors
    for (CSDFactorsCIter iter = actors.begin(); iter != actors.end(); iter++)
    {
        CSDFcomponent component = CSDFcomponent(g, g->nrActors());
        CSDFactor *a = (*iter)->clone(component);
        g->addActor(a);
    }
    
    // Channels
    for (CSDFchannelsCIter iter = channels.begin(); 
            iter != channels.end(); iter++)
    {
        CSDFcomponent component = CSDFcomponent(g, g->nrChannels());
        CSDFchannel *ch = (*iter)->clone(component);
        g->addChannel(ch);
    }
    
    return g;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF graph object.
 * All properties of the graph are copied.
 */
CSDFgraph *CSDFgraph::createCopy(CSDFcomponent &c) const
{
    CSDFgraph *g = new CSDFgraph(c);
    
    // Properties
    g->setName(getName());
    g->setType(getType());
    
    return g;
}

/**
 * construct ()
 * The function initializes all actor properties based on the XML data.
 */
void CSDFgraph::construct(const CNodePtr graphNode)
{
    CSDFgraph *g = this;
    
    // Name
    if (!CHasAttribute(graphNode, "name"))
        throw CException("Invalid CSDF graph, missing graph name.");
    g->setName(CGetAttribute(graphNode, "name"));

    // Type
    if (!CHasAttribute(graphNode, "type"))
        throw CException("Invalid CSDF graph, missing graph type.");
    g->setType(CGetAttribute(graphNode, "type"));
}

/**
 * getActor ()
 * The function returns a reference to an actor with the given id.
 */
CSDFactor *CSDFgraph::getActor(const CId id)
{
    for (CSDFactorsIter iter = actors.begin(); iter != actors.end(); iter++)
    {
        CSDFactor *a = *iter;
        
        if (a->getId() == id)
            return a;
    }
    
    throw CException("Graph '" + getName() + "' has no actor with id '"
                        + CString(id) + "'.");
}

/**
 * getActor ()
 * The function returns a reference to an actor with the given name.
 */
CSDFactor *CSDFgraph::getActor(const CString &name)
{
    for (CSDFactorsIter iter = actors.begin(); iter != actors.end(); iter++)
    {
        CSDFactor *a = *iter;
        
        if (a->getName() == name)
            return a;
    }
    
    throw CException("Graph '" + getName() + "' has no actor '" + name + "'.");
}

/**
 * addActor ()
 * Add an actor to a graph.
 */
void CSDFgraph::addActor(CSDFactor *a)
{
    actors.push_back(a);
}

/**
 * removeActor ()
 * Remove an actor from a graph.
 */
void CSDFgraph::removeActor(const CString &name)
{
    for (CSDFactorsIter iter = actors.begin(); iter != actors.end(); iter++)
    {
        CSDFactor *a = *iter;
        
        if (a->getName() == name)
        {
            actors.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() + "' has no actor '" + name + "'.");
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
CSDFactor *CSDFgraph::createActor()
{
    CSDFcomponent c = CSDFcomponent(this, nrActors());
    CSDFactor *a = new CSDFactor(c);
    a->setName(CString("_a") + CString(nrActors()+1));
    addActor(a);
    
    return a;
}

/**
 * createActor ()
 * Create a new actor on the graph.
 */
CSDFactor *CSDFgraph::createActor(CSDFcomponent &c)
{
    CSDFactor *a = new CSDFactor(c);
    addActor(a);
    
    return a;
}

/**
 * getChannel ()
 * The function returns a reference to a channel with the given id.
 */
CSDFchannel *CSDFgraph::getChannel(const CId id)
{
    for (CSDFchannelsIter iter = channels.begin(); 
            iter != channels.end(); iter++)
    {
        CSDFchannel *c = *iter;
        
        if (c->getId() == id)
            return c;
    }
    
    throw CException("Graph '" + getName() 
                        + "' has no channel with id '" + CString(id) + "'.");
}

/**
 * getChannel ()
 * The function returns a reference to a channel with the given name.
 */
CSDFchannel *CSDFgraph::getChannel(const CString &name)
{
    for (CSDFchannelsIter iter = channels.begin(); 
            iter != channels.end(); iter++)
    {
        CSDFchannel *c = *iter;
        
        if (c->getName() == name)
            return c;
    }
    
    throw CException("Graph '" + getName() 
                        + "' has no channel '" + name + "'.");
}

/**
 * addChannel ()
 * Add a channel to a graph.
 */
void CSDFgraph::addChannel(CSDFchannel *c)
{
    channels.push_back(c);
}

/**
 * removeChannel ()
 * Remove a channel from a graph.
 */
void CSDFgraph::removeChannel(const CString &name)
{
    for (CSDFchannelsIter iter = channels.begin(); 
            iter != channels.end(); iter++)
    {
        CSDFchannel *c = *iter;
        
        if (c->getName() == name)
        {
            delete c;
            channels.erase(iter);
            return;
        }
    }

    throw CException("Graph '" + getName() 
                        + "' has no channel '" + name + "'.");
}

/**
 * createChannel ()
 * Create a new channel on the graph.
 */
CSDFchannel *CSDFgraph::createChannel(CSDFcomponent &c)
{
    CSDFchannel *ch = new CSDFchannel(c);
    addChannel(ch);
    
    return ch;
}

/**
 * createChannel ()
 * The function creates a channel between the source and destination actor.
 * Ports with the supplied rates are added to these actors.
 */
CSDFchannel *CSDFgraph::createChannel(CSDFactor *src, CSDFrate rateSrc, 
        CSDFactor *dst, CSDFrate rateDst)
{
    // Create new channel
    CSDFcomponent c = CSDFcomponent(this, nrChannels());
    CSDFchannel *ch = createChannel(c);
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
 * isConnected ()
 * The function returns true if all actor ports are connected to a channel
 * and all channels are connected to a port. Else it returns false.
 */
bool CSDFgraph::isConnected() const
{
    for (CSDFactorsCIter iter = actors.begin(); iter != actors.end(); iter++)
    {
        const CSDFactor *a = *iter;
        
        if (!a->isConnected())
            return false;
    }
    
    for (CSDFchannelsCIter iter = channels.begin(); 
            iter != channels.end(); iter++)
    {
        const CSDFchannel *c = *iter;
        
        if (!c->isConnected())
            return false;
    }
    
    return true;
}

/**
 * isConsistent ()
 * The function returns true if the graph is consistent (i.e. the repetition
 * vector is non-zero). Else, it returns false.
 */
bool CSDFgraph::isConsistent()
{
    RepetitionVector vec = getRepetitionVector();
    
    if (vec[0] == 0)
        return false;
    
    return true;
}

/**
 * getRepetitionVector ()
 * The function calculates and returns the repetition vector of the graph.
 */
CSDFgraph::RepetitionVector CSDFgraph::getRepetitionVector()
{
    CFractions fractions(nrActors(), CFraction(0,1));
    uint ratePeriod = 1;
    
    // Compute period of repetition for rate vectors
    for (CSDFchannelsIter iter = channels.begin();
            iter != channels.end(); iter++)
    {
        CSDFchannel *c = *iter;
        
        ratePeriod = lcm(ratePeriod, (uint) c->getSrcPort()->getRate().size());
        ratePeriod = lcm(ratePeriod, (uint) c->getDstPort()->getRate().size());
    }
    
    // Calculate firing ratio (as fraction) for each actor
    for (CSDFactorsIter iter = actors.begin(); iter != actors.end(); iter++)
    {
        CSDFactor *a = *iter;
        CFraction &f = fractions[a->getId()];
        
        if (f == CFraction(0,1))
        {
            f = CFraction(1,1);
            calcFractionsConnectedActors(fractions, a, ratePeriod);
        }
    }
    
    // Calculate repetition vector based on firing ratios
    return calcRepetitionVector(fractions, ratePeriod);
}

/**
 * calcFractionsConnectedActors ()
 * The function calculates and firing ration (as fractions) of
 * all actors connected to actor 'a' based on its firing rate. In case
 * of an inconsistent graph, all fractions are set to 0.
 */
void CSDFgraph::calcFractionsConnectedActors(CFractions &fractions, 
        CSDFactor *a, uint ratePeriod)
{
    CFraction fractionA = fractions[a->getId()];

    // Inconsistent graph?
    if (fractionA == CFraction(0,0))
        return;

    // Calculate the rate for each actor 'b' connected to actor 'a'
    for (CSDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        CSDFport *pA = *iter;

        // Get actor 'b' on other side of channel and its port 'pB'
        CSDFchannel *c = pA->getChannel();
        CSDFport *pB = c->oppositePort(pA);
        CSDFactor *b = pB->getActor();

        // Tokens produced or consumed by actor A in one rate period
        uint rateA = 0;
        for (uint i = 0; i < pA->getRate().size(); i++)
            rateA += pA->getRate()[i];
        rateA = rateA * (ratePeriod / pA->getRate().size());

        // Tokens produced or consumed by actor B in one rate period
        uint rateB = 0;
        for (uint i = 0; i < pB->getRate().size(); i++)
            rateB += pB->getRate()[i];
        rateB = rateB * (ratePeriod / pB->getRate().size());

        // Graph inconsistent?
        if (rateA == 0 || rateB == 0)
        {
            // Inconsistent graph, set all fractions to 0
            for (uint i = 0; i < fractions.size(); i++)
                fractions[i] = CFraction(0,0);            

            return;
        }

        // Calculate firing rate 'b'
        CFraction ratioAB = CFraction(rateA, rateB);
        CFraction fractionB = fractionA * ratioAB;
        
        // Known firing rate for 'b'
        CFraction knownFractionB = fractions[b->getId()];

        // Compare known and calculated firing rate of 'b'
        if (knownFractionB != CFraction(0,1)
                && fractionB != knownFractionB)
        {
            // Inconsistent graph, set all fractions to 0
            for (uint i = 0; i < fractions.size(); i++)
                fractions[i] = CFraction(0,0);            
            
            return;
        }
        else if (knownFractionB == CFraction(0,1))
        {
            // Set the firing rate of actor 'b'
            fractions[b->getId()] = fractionB;
            
            // Calculate firing rate for all actors connnected to 'b'
            calcFractionsConnectedActors(fractions, b, ratePeriod);
            
            // Is graph inconsistent?
            if (fractions[b->getId()] == CFraction(0,0))
                return;
        }
    }
}

CSDFgraph::RepetitionVector CSDFgraph::calcRepetitionVector(
        CFractions &fractions, uint ratePeriod)
{
    RepetitionVector repetitionVector(fractions.size(), 0);
    long long int l = 1;
    
    // Find lowest common multiple (lcm) of all denominators
    for (CFractionsIter iter = fractions.begin(); 
            iter != fractions.end(); iter++)
    {
        CFraction &f = *iter;

        l = lcm(l, f.denominator());
    }
    
    // Zero vector?
    if (l == 0)
        return repetitionVector;
        
    // Calculate non-zero repetition vector
    for (uint i = 0; i < fractions.size(); i++)
    {
        repetitionVector[i] = 
                (fractions[i].numerator() * l) / fractions[i].denominator();
    }
    
    // Find greatest common divisor (gcd)
    int g = repetitionVector[0];
    for (uint i = 1; i < repetitionVector.size(); i++)
    {
        g = gcd(g, repetitionVector[i]);
    }

    // Minimize the repetition vector using the gcd
    for (uint i = 0; i < repetitionVector.size(); i++)
    {
        repetitionVector[i] = repetitionVector[i] / g;
    }
    
    // Multiply the repetition vector with the rate period
    for (uint i = 0; i < repetitionVector.size(); i++)
    {
        repetitionVector[i] = repetitionVector[i] * ratePeriod;
    }
    
    return repetitionVector;
}

/**
 * print ()
 * Print the graph to the supplied output stream.
 */
ostream &CSDFgraph::print(ostream &out)
{
    out << "Graph (" << getName() << ")" << endl;
    out << "id:        " << getId() << endl;
    out << "type:      " << getType() << endl;
    out << endl;

    for (CSDFactorsIter iter = actors.begin(); iter != actors.end(); iter++)
    {
        CSDFactor *a = *iter;
        
        a->print(out);
    }

    for (CSDFchannelsIter iter = channels.begin();
            iter != channels.end(); iter++)
    {
        CSDFchannel *c = *iter;
        
        c->print(out);
    }
    
    out << endl;
    
    return out;
}

/**
 * constructCSDFgraph ()
 * Construct an CSDF graph.
 */
CSDFgraph *constructCSDFgraph(const CNodePtr csdfNode)
{
    CId actorId = 0, channelId = 0, portId;
    CSDFgraph *g;
    
    // Construct CSDF graph
    CSDFcomponent component = CSDFcomponent(NULL, 0);
    g = new CSDFgraph(component);
    g->construct(csdfNode);
    
    // Construct all actors
    for (CNodePtr actorNode = CGetChildNode(csdfNode, "actor");
                actorNode != NULL; actorNode = CNextNode(actorNode, "actor"))
    {
        // Construct actor
        component = CSDFcomponent(g, actorId);
        CSDFactor *a = new CSDFactor(component);
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
            
            // Is the length of the rate sequence equal to the sequenceLength
            // of the actor?
            if (p->getRate().size() != a->sequenceLength())
                throw CException("[ERROR] length of rate sequences not equal.");
            
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
        CSDFchannel *c = new CSDFchannel(component);
        c->construct(channelNode);
 
        // Add channel to graph
        g->addChannel(c);
       
        // Next channel
        channelId++;
    }

    return g;
}
