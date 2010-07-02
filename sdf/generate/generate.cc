/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   generate.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 14, 2005
 *
 *  Function        :   Generate SDF graphs
 *
 *  History         :
 *      14-07-05    :   Initial version.
 *
 * $Id: generate.cc,v 1.7 2008/10/16 14:52:14 sander Exp $
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

#include "base/base.h"
#include "generate.h"
#include "../base/algo/cycle.h"
#include "../base/algo/graph.h"
#include "../analysis/analysis.h"
#include "../transform/hsdf/hsdf.h"
#include "../transform/model/buffersize.h"
#include "../transform/model/autoconc.h"

// Random number generator
static MTRand mtRand;

/**
 * relabelSDFgraph ()
 * Set new id's on all actors and channels starting from 0 and
 * change their names accordingly (actor<id> or ch<id>).
 */
void relabelSDFgraph(SDFgraph *g, CId *actorIdMap = NULL, 
        CId *channelIdMap = NULL)
{
    CId id;
    
    // Channels
    id = 0;
    for (SDFchannelsIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;

        if (channelIdMap != NULL)
            channelIdMap[id] = c->getId();

        c->setId(id);
        c->setName("ch" + CString(id));
        
        id++;
    }
    
    // Actors
    id = 0;
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *u = *iter;

        if (actorIdMap != NULL)
            actorIdMap[id] = u->getId();

        u->setId(id);
        u->setName("a" + CString(id));

        id++;
    }
}

/**
 * randomPort ()
 * The function returns an iterator to a random port in the list.
 */
SDFportsIter randomPort(SDFportsIter begin, uint size)
{
    uint n, i = 0;
    SDFportsIter iter = begin;

    ASSERT(size != 0, "Empty list of ports.");

    if (size > 1)
        n = mtRand.randInt(size-1);
    else
        n = 0;

    while (i < n) 
    {
        i++;
        iter++;
    }

    return iter;
}

/**
 * randomActor ()
 * The function returns an iterator to a random actor in the list.
 */
SDFactorsIter randomActor(SDFactorsIter begin, uint size)
{
    uint n, i = 0;
    SDFactorsIter iter = begin;

    ASSERT(size != 0, "Empty list of actors.");

    if (size > 1)
        n = mtRand.randInt(size-1);
    else
        n = 0;

    while (i < n) 
    {
        i++;
        iter++;
    }

    return iter;
}

/**
 * randomChannel ()
 * The function returns an iterator to a random channel in the list.
 */
SDFchannelsIter randomChannel(SDFchannelsIter begin, uint size)
{
    uint n, i = 0;
    SDFchannelsIter iter = begin;

    ASSERT(size != 0, "Empty list of channels.");

    if (size > 1)
        n = mtRand.randInt(size-1);
    else
        n = 0;

    while (i < n) 
    {
        i++;
        iter++;
    }

    return iter;
}

/**
 * createPort ()
 * Create a port with random rate on the actor.
 */
SDFport *createPort(SDFactor *a, const SDFport::SDFportType type, 
        const double avgRate, const double varRate, const double minRate, 
        const double maxRate)
{
    SDFport *p;
    SDFrate r;
        
    do 
    {
        r = (SDFrate)mtRand.randNorm(avgRate, varRate);
    } while (r < minRate || r > maxRate);
    
    p = a->createPort(type, r);
    p->setName("p" + CString(p->getId()));
    
    return p;
}

/**
 * createActor ()
 * Create an actor in the SDF graph.
 */
TimedSDFactor *createActor(TimedSDFgraph *g)
{
    TimedSDFactor::Processor *p;
    SDFcomponent component;
    TimedSDFactor *a;
    
    component = SDFcomponent(g, g->nrActors());
    a = g->createActor(component);
    a->setName("a" + CString(a->getId()));
    a->setType("A" + CString(a->getId()));

    // Add a default processor with unit execution time
    p = a->addProcessor("default");
    p->execTime = 1;
    a->setDefaultProcessor("default");
    
    return a;
}

/**
 * createChannel ()
 * Create a channel in the SDF graph from pi to po.
 */
TimedSDFchannel *createChannel(TimedSDFgraph *g, SDFport *pi, SDFport *po)
{
    SDFcomponent component;
    TimedSDFchannel *c;
    
    component = SDFcomponent(g, g->nrChannels());
    c = g->createChannel(component);
    c->setName("ch" + CString(c->getId()));
    c->connectSrc(po);
    c->connectDst(pi);
    
    return c;
}

/**
 * createGraph ()
 * Create a random, connected SDF graph.
 */
TimedSDFgraph *createGraph(const uint nrActors, 
        const double avgInDegree, const double varInDegree, 
        const double minInDegree, const double maxInDegree,
        const double avgOutDegree, const double varOutDegree, 
        const double minOutDegree, const double maxOutDegree,
        const double avgRate, const double varRate,
        const double minRate, const double maxRate)
{
    SDFcomponent component;
    TimedSDFgraph *g;
    TimedSDFactor *a;
    SDFport *pi, *po;
    SDFports inPorts, outPorts;
    SDFportsIter piIter, poIter;
    
    // Create an SDF graph
    component = SDFcomponent(NULL,0);
    g = new TimedSDFgraph(component);
    g->setName("g");
    g->setType("G");

    // Create graph with nrActors actors
    while (g->nrActors() < nrActors)
    {
        SDFports inPortsActor, outPortsActor;
        uint id, od;
        
        // Add a new actor to the graph
        a = createActor(g);
        
        // Number of input and output ports of actor a
        do
        {
            id = (uint)mtRand.randNorm(avgInDegree, varInDegree);
        } while (id < minInDegree || id > maxInDegree);

        do
        {
            od = (uint)mtRand.randNorm(avgOutDegree, varOutDegree);
        } while (od < minOutDegree || od > maxOutDegree);
        
        // Create input ports with random rates
        while (a->nrPorts() < id)
        {
            pi = createPort(a, SDFport::In, avgRate, varRate, minRate, maxRate);
            inPortsActor.push_back(pi);
        }
        
        // Create output ports with random rates
        while (a->nrPorts() < id + od)
        {
            po = createPort(a, SDFport::Out, avgRate, varRate, minRate,maxRate);
            outPortsActor.push_back(po);
        }

        // Not first actor in the graph?
        if (g->nrActors() > 1)
        {
                
            // Create a channel between the new actor and an existing actor.
            // This guarantees connectedness of the SDF graph.
            if (mtRand.rand() < 0.5)
            {
                // Select random input port on actor a
                piIter = randomPort(inPortsActor.begin(), inPortsActor.size());
                pi = *piIter;

                // Select random output port on set of output ports
                poIter = randomPort(outPorts.begin(), outPorts.size());
                po = *poIter;

                // Create a channel
                createChannel(g, pi, po);

                // Remove input and output port from their lists
                inPortsActor.erase(piIter);
                outPorts.erase(poIter);
            }
            else
            {
                // Select random output port on actor a
                poIter = randomPort(outPortsActor.begin(),outPortsActor.size());
                po = *poIter;

                // Select random input port on set of input ports
                piIter = randomPort(inPorts.begin(), inPorts.size());
                pi = *piIter;

                // Create a channel
                createChannel(g, pi, po);

                // Remove input and output port from their lists
                inPorts.erase(piIter);
                outPortsActor.erase(poIter);
            }
        }
        
        // Add unconnected ports to the inPorts and outPorts lists
        inPorts.insert(inPorts.end(), inPortsActor.begin(), inPortsActor.end());
        outPorts.insert(outPorts.end(), outPortsActor.begin(),
                            outPortsActor.end());
    }

    // Create channels between the input and output ports of the actors
    while (inPorts.size() != 0 && outPorts.size() != 0)
    {
        // Random input and output port
        piIter = randomPort(inPorts.begin(), inPorts.size());
        poIter = randomPort(outPorts.begin(), outPorts.size());
        pi = *piIter;
        po = *poIter;
        
        // Create a channel
        createChannel(g, pi, po);

        // Remove input and output port from their lists
        inPorts.erase(piIter);
        outPorts.erase(poIter);
    }

    // Remove all unconnected input ports
    if (inPorts.size() != 0)
    {
        for (piIter = inPorts.begin(); piIter != inPorts.end(); piIter++)
        {
            pi = *piIter;
            a = (TimedSDFactor*)pi->getActor();
            a->removePort(pi->getName());   
        }
    }
        
    // Remove all unconnected output ports
    if (outPorts.size() != 0)
    {
        for (poIter = outPorts.begin(); poIter != outPorts.end(); poIter++)
        {
            po = *poIter;
            a = (TimedSDFactor*)po->getActor();
            a->removePort(po->getName());   
        }
    }

    return g;    
}

/**
 * createAcyclicGraph ()
 * Create a random, connected, acyclic SDF graph.
 */
TimedSDFgraph *createAcyclicGraph(const uint nrActors, 
        const double avgInDegree, const double varInDegree, 
        const double minInDegree, const double maxInDegree,
        const double avgOutDegree, const double varOutDegree, 
        const double minOutDegree, const double maxOutDegree,
        const double avgRate, const double varRate,
        const double minRate, const double maxRate)
{
    SDFcomponent component;
    TimedSDFgraph *g;
    TimedSDFactor *a;
    SDFport *pi, *po;
    SDFports inPorts, outPorts;
    SDFportsIter piIter, poIter;
    
    // Create an SDF graph
    component = SDFcomponent(NULL,0);
    g = new TimedSDFgraph(component);
    g->setName("g");
    g->setType("G");

    // Create graph with nrActors actors
    while (g->nrActors() < nrActors)
    {
        SDFports inPortsActor, outPortsActor;
        uint id, od;
        
        // Add a new actor to the graph
        a = createActor(g);
        
        // Number of input and output ports of actor a
        do
        {
            id = (uint)mtRand.randNorm(avgInDegree, varInDegree);
        } while (id < minInDegree || id > maxInDegree);

        do
        {
            od = (uint)mtRand.randNorm(avgOutDegree, varOutDegree);
        } while (od < minOutDegree || od > maxOutDegree);
        
        // Create input ports with random rates
        while (a->nrPorts() < id)
        {
            pi = createPort(a, SDFport::In, avgRate, varRate, minRate, maxRate);
            inPortsActor.push_back(pi);
        }
        
        // Create output ports with random rates
        while (a->nrPorts() < id + od)
        {
            po = createPort(a, SDFport::Out, avgRate, varRate, minRate,maxRate);
            outPortsActor.push_back(po);
        }

        // Not first actor in the graph?
        if (g->nrActors() > 1)
        {        
            // Create a channel between the new actor and an existing actor.
            // This guarantees connectedness of the SDF graph.
            // Actors can only connect to actor with a higher id. So, 
            // the actor a must be reached by an actor already created 
            // (has lower id).

            // Select random input port on actor a
            piIter = randomPort(inPortsActor.begin(), inPortsActor.size());
            pi = *piIter;

            // Select random output port on set of output ports
            poIter = randomPort(outPorts.begin(), outPorts.size());
            po = *poIter;

            // Create a channel
            createChannel(g, pi, po);

            // Remove input and output port from their lists
            inPortsActor.erase(piIter);
            outPorts.erase(poIter);
        }
        
        // Add unconnected ports to the inPorts and outPorts lists
        inPorts.insert(inPorts.end(), inPortsActor.begin(), inPortsActor.end());
        outPorts.insert(outPorts.end(), outPortsActor.begin(),
                            outPortsActor.end());
    }

    // Create channels between the input and output ports of the actors
    while (inPorts.size() != 0 && outPorts.size() != 0)
    {
        // Select random output port
        poIter = randomPort(outPorts.begin(), outPorts.size());
        po = *poIter;        

        // Select random input port on actor with higher id
        uint sz = inPorts.size();
        piIter = inPorts.begin();
        while (piIter != inPorts.end() && 
                    (*piIter)->getActor()->getId() <= po->getActor()->getId())
        {
            piIter++;
            sz--;
        }
        
        if (piIter != inPorts.end())
        {
            piIter = randomPort(piIter, sz);
            pi = *piIter;

            // Create a channel
            createChannel(g, pi, po);

            // Remove input port from its list
            inPorts.erase(piIter);
        }
        else
        {
            // This output port will never be connected, remove it
            po->getActor()->removePort(po->getName());
        }
        
        // Remove output port from its list
        outPorts.erase(poIter);
    }

    // Remove all unconnected input ports
    if (inPorts.size() != 0)
    {
        for (piIter = inPorts.begin(); piIter != inPorts.end(); piIter++)
        {
            pi = *piIter;
            a = (TimedSDFactor*)pi->getActor();
            a->removePort(pi->getName());   
        }
    }
        
    // Remove all unconnected output ports
    if (outPorts.size() != 0)
    {
        for (poIter = outPorts.begin(); poIter != outPorts.end(); poIter++)
        {
            po = *poIter;
            a = (TimedSDFactor*)po->getActor();
            a->removePort(po->getName());   
        }
    }

    return g;    
}

/**
 * makeConsistentConnectedActors ()
 * The function calculates firing rates (as fractions) of
 * all actors connected to actor 'a' based on its firing rate. In case
 * of an inconsistent graph, the inconsistency is fixed by adjusting the rate
 * of a port.
 */
void makeConsistentConnectedActors(CFractions &fractions, SDFactor *a)
{
    CFraction fractionA = fractions[a->getId()];

    // Calculate the rate for each actor 'b' connected to actor 'a'
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *pA = *iter;

        // Get actor 'b' on other side of channel and its port 'pB'
        SDFchannel *c = pA->getChannel();
        SDFport *pB = c->getSrcPort();
        SDFactor *b = pB->getActor();
        if (a->getId() == b->getId())
        { 
            pB = c->getDstPort(); 
            b = pB->getActor();
        }

        // Calculate firing rate 'b'
        CFraction ratioAB = CFraction(pA->getRate(), pB->getRate());
        CFraction fractionB = fractionA * ratioAB;
        
        // Known firing rate for 'b'
        CFraction knownFractionB = fractions[b->getId()];

        // Compare known and calculated firing rate of 'b'
        if (knownFractionB != CFraction(0,1)
                && fractionB != knownFractionB)
        {
            // Inconsistent graph, fix the rate on actors a and b
            ratioAB = fractionA / knownFractionB;
            
            pB->setRate(ratioAB.numerator());
            pA->setRate(ratioAB.denominator());
        }
        else if (knownFractionB == CFraction(0,1))
        {
            // Set the firing rate of actor 'b'
            fractions[b->getId()] = fractionB;
            
            // Calculate firing rate for all actors connnected to 'b'
            makeConsistentConnectedActors(fractions, b);
        }
    }
}

/**
 * makeConsistent ()
 * The rates on the ports are adjuested to create a consistent SDF graph.
 */
void makeConsistent(SDFgraph *g)
{
    CFractions fractions(g->nrActors(), CFraction(0,1));

    // Calculate firing ratio (as fraction) for each actor
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        CFraction &f = fractions[a->getId()];
        
        if (f == CFraction(0,1))
        {
            f = CFraction(1,1);
            makeConsistentConnectedActors(fractions, a);
        }
    }
}

/**
 * makeStronglyConnected ()
 * The function changes the SDF graph into a strongly connected SDF
 * graph by selecting the largest strongly connected component in the
 * graph and removing actors and channels which do not belong to
 * this component from the graph.
 */
void makeStronglyConnected(SDFgraph *g)
{
    SDFgraphComponents comps;
    SDFgraphComponent comp;
    uint maxSize = 0;
    
    // Get a list of all strongly connected components
    comps = stronglyConnectedComponents(g);

    // Find the largest component
    for (SDFgraphComponentsIter iter = comps.begin();
            iter!= comps.end(); iter++)
    {
        if (maxSize < (*iter).size())
        {
            comp = *iter;
            maxSize = comp.size();
        }
    }
    
    // Remove all channels not in the component (source and/or destination not
    // in component)
    for (SDFchannelsIter iter = g->channelsBegin(); 
            iter != g->channelsEnd();)
    {
        SDFchannel *c = *iter;
        SDFactor *u = c->getSrcActor();
        SDFactor *v = c->getDstActor();

        // Advance iterator
        iter++;
        
        if (!actorInComponent(u, comp) || !actorInComponent(v, comp))
        {
            // Remove port at both sides
            u->removePort(c->getSrcPort()->getName());
            v->removePort(c->getDstPort()->getName());
            
            // Remove channel c
            g->removeChannel(c->getName());
        }
    }

    // Remove all actors not in the component
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd();)
    {
        SDFactor *u = *iter;

        // Advance iterator
        iter++;
        
        if (!actorInComponent(u, comp))
            g->removeActor(u->getName());
    }
    
    // Relabel the actors and channels
    relabelSDFgraph(g);
    
    if (g->nrChannels() == 0)
        throw CException("Component contain no edges.");

    if (!isStronglyConnectedGraph(g))
        throw CException("Graph is not strongly connected");
}

/**
 * fireActor ()
 * The function removes the number of tokens required for a firing of the actor
 * from all input ports and it produces tokens on all output ports.
 */
static
void fireActor(SDFactor *a)
{
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        
        if (p->getType() == SDFport::In)
        {
            if (c->getInitialTokens() < p->getRate())
            {
                cerr << "actor: " << a->getId() << endl;
                throw CException("Not enough tokens to fire actor.");
            }

            c->setInitialTokens(c->getInitialTokens() - p->getRate());
        }
        else
        {
            c->setInitialTokens(c->getInitialTokens() + p->getRate());
        }
    }
}

/**
 * isActorReady ()
 * The function checks wether there are enough tokens on the input ports
 * to fire the actor.
 */
bool isActorReady(SDFactor *a)
{
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        
        if (p->getType() == SDFport::In)
        {
            SDFchannel *c = p->getChannel();
            
            if (c->getInitialTokens() < p->getRate())
                return false;
        }
    }
    
    return true;
}

/**
 * updateActorReadyList ()
 * The function updates the list of ready actors after the firing of actor a.
 */
void updateActorReadyList(SDFactors &readyList, SDFactor *a)
{
    // Is actor a no longer ready?
    if (!isActorReady(a))
    {
        // Remove actor a from the ready list
        for (SDFactorsIter iter = readyList.begin();
                iter != readyList.end(); iter++)
        {
            if ((*iter)->getId() == a->getId())
            {
                readyList.erase(iter);
                break;
            }
        }
    }
    
    // Which actors are ready because actor a fired?
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        
        if (p->getType() == SDFport::Out)
        {
            SDFactor *b = p->getChannel()->getDstActor();
            
            if (isActorReady(b))
            {
                bool alreadyListed = false;
                
                // Add actor only when not yet in the list
                for (SDFactorsIter iter = readyList.begin();
                        iter != readyList.end() && !alreadyListed; iter++)
                {
                    if ((*iter)->getId() == b->getId())
                        alreadyListed = true;
                }
                if (!alreadyListed)
                    readyList.push_back(b);
            }
        }
    }
}

/**
 * getRandomActorFromActorReadyList ()
 * The function returns a random actor from the list.
 */
SDFactor *getRandomActorFromActorReadyList(SDFactors &readyList)
{
    int n;
    
    // Select a random actor from the list
    n = mtRand.randInt(readyList.size()-1);
    
    for (SDFactorsIter iter = readyList.begin();
            iter != readyList.end(); iter++)
    {
        if (n == 0)
            return *iter;
        n--;
    }
    
    return NULL;
}

/**
 * execSDFgraph ()
 * Fire the actors in the SDF graph. The number of firings is n (or less in case
 * of deadlock).
 */
bool execSDFgraph(SDFgraph *g, uint n)
{
    SDFactors readyList;
    SDFactor *a;
    
    // Create initial list of ready actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        a = *iter;
    
        if (isActorReady(a))
            readyList.push_back(a);
    }
    
    // Firings
    while (n > 0 && !readyList.empty())
    {
        // Get a random actor from the list of ready actors
        a = getRandomActorFromActorReadyList(readyList);
        ASSERT(a != NULL, "ready list is empty");
        
        // Fire the actor
        fireActor(a);
        
        // Update the list of ready actors
        updateActorReadyList(readyList, a);
    
        // Next
        n--;
    }
    
    // Deadlock?
    if (readyList.empty())
        return false;
    
    return true;
}

/**
 * distributeInitialTokensInComponent ()
 * Add initial tokens to the strongly connected component to 
 * guarantee a deadlock-free execution of the SDF component
 */
void distributeInitialTokensInComponent(TimedSDFgraph *g,
        SDFgraphComponent &component, const double initialTokenProp)
{
    RepetitionVector repVec;
    CId *actorIdMap, *channelIdMap;
    SDFgraphCycle cycle;
    TimedSDFgraph *gr;
    SDFchannel *c, *cg;
    SDFactor *a, *b;
    SDFport *p;
    uint n = 0;
    
    // Transform component to SDF graph
    gr = (TimedSDFgraph*)componentToSDFgraph(component);
    actorIdMap = new CId [gr->nrActors()];
    channelIdMap = new CId [gr->nrChannels()];
    relabelSDFgraph(gr, actorIdMap, channelIdMap);

    //No channels in component?
    if (gr->nrChannels() == 0)
    {
        // Cleanup
        delete gr;
        delete [] actorIdMap;
        delete [] channelIdMap;

        // Done
        return;
    }

    // Break each cycle in the SDFG by adding enough initial tokens to at least
    // one channel to complete a full iteration of the SDFG (component).
    repVec = computeRepetitionVector(gr);
    cycle = findSimpleCycle(gr);
    while (cycle.size() != 0)
    {
        // Get first two actors of the cycle
        if (cycle.size() == 1)
        {
            a = *(cycle.begin());
            b = a;
        }
        else
        {
            a = *(cycle.begin());
            b = *(++(cycle.begin()));
        }

        // Add initial tokens to all channels from a to b
        for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd();)
        {
            p = *iter;
            
            // Advance iterator
            iter++;
            
            if (p->getType() == SDFport::Out)
            {
                c = p->getChannel();
                
                if (c->oppositePort(p)->getActor()->getId() == b->getId())
                {
                    cg = g->getChannel(channelIdMap[c->getId()]);
                    cg->setInitialTokens(repVec[a->getId()] * p->getRate());

                    b->removePort(c->oppositePort(p)->getName());
                    a->removePort(p->getName());
                    gr->removeChannel(c->getName());
                }
            }
        }
        
        // Find next cycle in the graph
        cycle = findSimpleCycle(gr);
    }

    // Cleanup
    delete gr;
    delete [] actorIdMap;
    delete [] channelIdMap;
    
    // Transform component again to SDF graph
    gr = (TimedSDFgraph*)componentToSDFgraph(component);
    actorIdMap = new CId [gr->nrActors()];
    channelIdMap = new CId [gr->nrChannels()];
    relabelSDFgraph(gr, actorIdMap, channelIdMap);

    // Execute random, enabled actors
    repVec = computeRepetitionVector(gr);
    for (uint i = 0; i < gr->nrActors(); i++)
        n += repVec[i];
    execSDFgraph(gr, mtRand.randInt(10*n));
    
    // Update initial tokens in the original SDF graph
    for (SDFchannelsIter iter = gr->channelsBegin();
            iter != gr->channelsEnd(); iter++)
    {
        c = *iter;
        cg = g->getChannel(channelIdMap[c->getId()]);
        cg->setInitialTokens(c->getInitialTokens());
    }

    // Cleanup
    delete gr;
    delete [] actorIdMap;
    delete [] channelIdMap;
}

/**
 * distributeInitialTokens ()
 * Add initial tokens to the strongly connected components in the SDF to 
 * guarantee a deadlock-free SDF graph.
 */
void distributeInitialTokens(TimedSDFgraph *g, const double initialTokenProp)
{
    SDFgraphComponents components;
    SDFchannel *c;
 
    // Create deadlock-free strongly connected components
    components = stronglyConnectedComponents(g);
    for (SDFgraphComponentsIter iter = components.begin();
            iter != components.end(); iter++)
    {
        SDFgraphComponent comp = *iter;
        
        distributeInitialTokensInComponent(g, comp, initialTokenProp);
    }

    // Graph is deadlock free, let's add some more tokens to it
    while (mtRand.rand() < initialTokenProp)
    {
        // Add token to random channel
        c = *(randomChannel(g->channelsBegin(), g->nrChannels()));
        c->setInitialTokens(c->getInitialTokens() + 1);
    };
}

/**
 * assignConsistentRates ()
 * The function attaches rates to all ports of the SDF graph that enforce
 * that the sum of the repetition vector entries is equal to the repVecSum.
 */
void assignConsistentRates(SDFgraph *g, const uint repVecSum)
{
    vector<uint> repVec(g->nrActors());
    uint remainingSum, scale;
    
    // Repetition vector sum should be at least large enough to create an HSDFG
    if (repVecSum < g->nrActors())
        throw CException("[ERROR] Repetition vector sum too small.");
        
    // Construct a random repetition vector with the sum equal to the supplied
    // value
    remainingSum = repVecSum;
    for (uint i = g->nrActors() - 1; i > 0; i--)
    {
        do 
        { 
            repVec[i] = mtRand.randInt(remainingSum - i); 
        } while (repVec[i] == 0);
        
        remainingSum -= repVec[i];
    }
    repVec[0] = remainingSum;

    // Assign rate to every port
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        SDFactor *srcActor = c->getSrcActor();
        SDFactor *dstActor = c->getDstActor();
        SDFport *srcPort = c->getSrcPort();
        SDFport *dstPort = c->getDstPort();
        SDFrate srcRate, dstRate;
        
        // Initially the rates are equal to the repetition vector entries
        // of the opposite actor
        srcRate = repVec[dstActor->getId()];
        dstRate = repVec[srcActor->getId()];
        
        // Scale the rates with the gcd of the rates
        scale = gcd(srcRate, dstRate);
        srcRate = srcRate / scale;
        dstRate = dstRate / scale;
        
        // Assign rate to ports
        srcPort->setRate(srcRate);
        dstPort->setRate(dstRate);
    }
}

/**
 * makeSimpleGraph ()
 * The function transforms the multigraph g to a graph in which there is at most
 * one channel from a node to another node. When multiple channels are found,
 * all except the channel that was found first are removed.
 */
void makeSimpleGraph(SDFgraph *g)
{
    // Iterate over all ports on all actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        vector<bool> connected(g->nrActors(), false);
        
        // Iterate over all ports of actor a
        for (SDFportsIter iterP = a->portsBegin(); 
                iterP != a->portsEnd();)
        {
            SDFport *p = *iterP;
            SDFchannel *c = p->getChannel();
            SDFactor *oppositeActor = c->oppositePort(p)->getActor();
            
            // Advance iterator to next port
            iterP++;
            
            // Already seen a channel from a to this actor
            if (connected[oppositeActor->getId()])
            {
                // Remove the channel and associated ports
                oppositeActor->removePort(c->oppositePort(p)->getName());
                a->removePort(p->getName());
                g->removeChannel(c->getName());
            }
            else
            {
                // Mark the opposite actor as connected to actor a
                connected[oppositeActor->getId()] = true;
            }
        }
    }

    // Relabel the channels
    relabelSDFgraph(g);
}

/**
 * generateSDFgraph ()
 * Generate a random SDF graph.
 */
TimedSDFgraph *generateSDFgraph(const uint nrActors, 
        const double avgInDegree, const double varInDegree, 
        const double minInDegree, const double maxInDegree,
        const double avgOutDegree, const double varOutDegree, 
        const double minOutDegree, const double maxOutDegree,
        const double avgRate, const double varRate,
        const double minRate, const double maxRate, 
        const bool acyclic, const bool stronglyConnected,
        const double initialTokenProp, const uint repetitionVectorSum,
        const bool multigraph)
{
    TimedSDFgraph *g;

    if (acyclic)
    {
        g = createAcyclicGraph(nrActors, avgInDegree, varInDegree, 
                    minInDegree, maxInDegree, avgOutDegree, varOutDegree, 
                    minOutDegree, maxOutDegree, avgRate, varRate, minRate,
                    maxRate);
       
        if (repetitionVectorSum != 0)
            assignConsistentRates(g, repetitionVectorSum);
        else
            makeConsistent(g);
 
        if (stronglyConnected)
            throw CException("Acyclic graph cannot be strongly connected.");    
    }
    else
    {
        g = createGraph(nrActors, avgInDegree, varInDegree, 
                    minInDegree, maxInDegree, avgOutDegree, varOutDegree, 
                    minOutDegree, maxOutDegree, avgRate, varRate, minRate,
                    maxRate);
        
        if (repetitionVectorSum != 0)
            assignConsistentRates(g, repetitionVectorSum);
        else
            makeConsistent(g);
        
        if (stronglyConnected)
            makeStronglyConnected(g);
    
        distributeInitialTokens(g, initialTokenProp);
    }
    
    // No multigraph?
    if (!multigraph)
        makeSimpleGraph(g);
    
    return g;
}

/**
 * randomActorExecTime ()
 * The function assigns a random execution time to the actor.
 */
SDFtime randomActorExecTime(const double avgExecTime, const double varExecTime,
        const double minExecTime, const double maxExecTime)
{
    SDFtime t;

    do
        t = (SDFtime)mtRand.randNorm(avgExecTime, varExecTime);   
    while ( t < minExecTime || t > maxExecTime);
    
    return t;
}

/**
 * randomActorStateSize ()
 * The function assigns a random state size to the actor.
 */
CSize randomActorStateSize(const double avgStateSize, const double varStateSize,
        const double minStateSize, const double maxStateSize)
{
    uint sz;

    do
        sz = (uint)mtRand.randNorm(avgStateSize, varStateSize);
    while ( sz < minStateSize || sz > maxStateSize);

    return sz;
}

/**
 * randomChannelTokenSize ()
 * The function assigns a random token size to the tokens in the channel.
 */
void randomChannelTokenSize(TimedSDFchannel *c, 
        const double avgTokenSize, const double varTokenSize,
        const double minTokenSize, const double maxTokenSize)
{
    uint sz;

    do
        sz = (uint)mtRand.randNorm(avgTokenSize, varTokenSize);
    while ( sz < minTokenSize || sz > maxTokenSize);

    c->setTokenSize(sz);
}

/**
 * randomChannelBufferSizes ()
 * Assign a random buffer sizes to all channels. The function guarantees
 * that the buffers are large enough to realize a positive (non-zero) 
 * throughput.
 */
void randomChannelBufferSizes(TimedSDFgraph *g)
{
    SDFstateSpaceBufferAnalysis bufferAnalysisAlgo;
    RepetitionVector repVec;
    StorageDistributionSet *minStorageDist;
    TimedSDFchannel::BufferSize bufferSize;
    StorageDistribution *d;
    TimedSDFchannel *c;
    uint minSz, maxSz;

    // Compute the minimal storage distribution which has a positive throughput
    minStorageDist = bufferAnalysisAlgo.analyze(g, 0);

    while (minStorageDist != NULL && minStorageDist->thr == 0)
    {
        minStorageDist = minStorageDist->next;
    }
    
    if (minStorageDist == NULL || minStorageDist->distributions == NULL)
    {
        throw CException("No minimal storage distribution found.");
    }
        
    // Select first storage distribution with positive throughput
    d = minStorageDist->distributions;

    // Set buffer size of each channel to bound given by distribution d
    for (SDFchannelsIter iter = g->channelsBegin();
        iter != g->channelsEnd(); iter++)
    {
        c = (TimedSDFchannel*)*iter;

        bufferSize.sz = d->sp[c->getId()];
        c->setBufferSize(bufferSize);
    }

    // Compute repetition vector of the graph
    repVec = computeRepetitionVector(g);

    // Add random buffer space to channels
    for (SDFchannelsIter iter = g->channelsBegin();
        iter != g->channelsEnd(); iter++)
    {
        c = (TimedSDFchannel*)*iter;
        bufferSize = c->getBufferSize();

        // Minimum buffer size
        minSz = bufferSize.sz;

        // Compute maximum buffer size        
        maxSz = repVec[c->getSrcActor()->getId()] * c->getSrcPort()->getRate()
               + repVec[c->getDstActor()->getId()] * c->getDstPort()->getRate();
        maxSz = (maxSz > c->getInitialTokens() ? maxSz : c->getInitialTokens());
        maxSz = (maxSz > minSz ? maxSz : minSz);
        
        // Choose random buffer size between lower and upper bound
        bufferSize.sz = minSz + mtRand.randInt(maxSz-minSz);

        // Set buffer size of the channel
        c->setBufferSize(bufferSize);
    }

    // Compute buffer space when mapped to connection or tile memory
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        c = (TimedSDFchannel*)*iter;
        bufferSize = c->getBufferSize();

        // Buffer mapped to memory
        bufferSize.mem = bufferSize.sz;
        if (bufferSize.mem < (int)c->getInitialTokens())
            bufferSize.mem  = c->getInitialTokens();
        
        // Buffer mapped to connection (source)
        bufferSize.src = (bufferSize.sz / 2) + 1;
        if (bufferSize.src < (int)c->getInitialTokens())
            bufferSize.src = c->getInitialTokens();
        if (bufferSize.src < (int)c->getSrcPort()->getRate())
            bufferSize.src = c->getSrcPort()->getRate();

        // Buffer mapped to connection (destination)
        bufferSize.dst = (bufferSize.sz / 2) + 1;
        if (bufferSize.dst < (int)c->getDstPort()->getRate())
            bufferSize.dst = c->getDstPort()->getRate();

        // Set buffer size of channel c
        c->setBufferSize(bufferSize);
    }
}

/**
 * randomBandwidthRequirement ()
 * The function assigns a random bandwidth requirement to the channel.
 */
void randomBandwidthRequirement(TimedSDFchannel *c, const double avgBandwidth, 
        const double varBandwidth, double minBandwidth, double maxBandwidth)
{
    double bw;

    do
        bw = mtRand.randNorm(avgBandwidth, varBandwidth);
    while (bw < minBandwidth || bw > maxBandwidth);
   
    c->setMinBandwidth(bw);
}

/**
 * randomLatencyRequirement ()
 * The function assigns a random latency requirement to the channel.
 */
void randomLatencyRequirement(TimedSDFchannel *c, const double avgLatency, 
        const double varLatency, double minLatency, double maxLatency)
{
    SDFtime l;

    do
        l = (SDFtime)mtRand.randNorm(avgLatency, varLatency);
    while (l < minLatency || l > maxLatency);
   
    c->setMinLatency(l);
}

/**
 * randomThroughputConstraint ()
 * Set a random throughput constraint on the graph.
 */
void randomThroughputConstraint(TimedSDFgraph *g,
        const bool bufferSize, const uint autoConcurrencyDegree,
        const double throughputScaleFactor)
{
    SDFstateSpaceThroughputAnalysis thrAnalysisAlgo;
    TimedSDFgraph *ga, *gb;
    SDFcomponent component;
    double thr;

    // Auto-concurrency limited?
    if (autoConcurrencyDegree != 0)
    {
        ga = (TimedSDFgraph*)modelAutoConcurrencyInSDFgraph(g,
                                            autoConcurrencyDegree);
    }
    else
    {
        component = SDFcomponent(g->getParent(), g->getId(), g->getName());
        ga = g->clone(component);
    }

    // Buffer size channels limited?
    if (bufferSize)
    {
        gb = modelBufferSizeInSDFgraph(ga);
        thr = thrAnalysisAlgo.analyze(gb);
        delete gb;
    }
    else
    {
        thr = thrAnalysisAlgo.analyze(ga);
    }

    // Set throughput constraint
    g->setThroughputConstraint(throughputScaleFactor*mtRand.rand()*thr);

    // Cleanup
    delete ga;
}

/**
 * generateSDFgraphProperties ()
 * Generate properties for a random SDF graph.
 */
void generateSDFgraphProperties(TimedSDFgraph *g,
        const bool execTime, const uint nrProcTypes, 
        const double mapChance, const double avgExecTime,
        const double varExecTime, const double minExecTime, 
        const double maxExecTime, const bool stateSize,
        const double avgStateSize, const double varStateSize, 
        const double minStateSize, const double maxStateSize,
        const bool tokenSize, const double avgTokenSize, 
        const double varTokenSize, const double minTokenSize, 
        const double maxTokenSize, const bool throughputConstraint,
        const uint autoConcurrencyDegree, const double throughputScaleFactor,
        const bool bandwidthRequirement, const double avgBandwidth,
        const double varBandwidth, const double minBandwidth,
        const double maxBandwidth, const bool bufferSize,
        const bool latencyRequirement, const double avgLatency,
        const double varLatency, const double minLatency,
        const double maxLatency, const bool integerMCM)
{
    SDFstateSpaceThroughputAnalysis thrAnalysisAlgo;
    TimedSDFactor::Processor *p;
    TimedSDFactor *a;
    TimedSDFchannel *c;
    double mcm, thr;

    do
    {
        // Actor properties
        for (SDFactorsIter iter = g->actorsBegin(); 
                iter != g->actorsEnd(); iter++)
        {
            bool isMapped = false;
            a = (TimedSDFactor*)(*iter);

            // Remove all processor currently connected to the actor
            for (TimedSDFactor::ProcessorsIter iterP = a->processorsBegin();
                    iterP != a->processorsEnd();)
            {
                TimedSDFactor::Processor *p = *iterP;
                iterP++;
                a->removeProcessor(p->type);
            }

            // Execution time
            while (!isMapped)
            {
                for (uint procType = 0; procType < nrProcTypes; procType++)
                {
                    // Does actor a map on this processor type?
                    if (mtRand.rand() < mapChance)
                    {
                        // Add processor to the actor
                        p = a->addProcessor("proc_" + CString(procType));

                        if (execTime)
                        {
                            // Remove processor named 'default'
                            a->removeProcessor("default");
                        
                            p->execTime = randomActorExecTime(avgExecTime,
                                    varExecTime, minExecTime, maxExecTime);
                        }

                        if (stateSize)
                        {
                            p->stateSize = randomActorStateSize(avgStateSize,
                                    varStateSize, minStateSize, maxStateSize);
                        }

                        // First processor is the default
                        if (!isMapped)
                            a->setDefaultProcessor(p->type);

                        isMapped = true;
                    }
                }
            }
        }
    
        // Integer valued MCM requested?
        if (integerMCM)
        {
            thr = thrAnalysisAlgo.analyze(g);
            mcm = 1.0 / thr;
            ASSERT(thr != 0, "Deadlock - insufficient tokens!");
        }
        else
        {
            mcm = 1;
        }
    }
    while (integerMCM && mcm != (double)((int)(mcm)));
   
    // Channel properties
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        c = (TimedSDFchannel*)*iter;

        if (tokenSize)
            randomChannelTokenSize(c, avgTokenSize, varTokenSize, 
                    minTokenSize, maxTokenSize);
    }
    
    if (bufferSize)
        randomChannelBufferSizes(g);

    // Throughput constraint
    if (throughputConstraint)
    {
        randomThroughputConstraint(g, bufferSize, 
                            autoConcurrencyDegree, throughputScaleFactor);
    }
    
    // Bandwidth requirement
    if (bandwidthRequirement)
    {
        for (SDFchannelsIter iter = g->channelsBegin();
                iter != g->channelsEnd(); iter++)
        {
            c = (TimedSDFchannel*)*iter;

            randomBandwidthRequirement(c, avgBandwidth, varBandwidth,
                    minBandwidth, maxBandwidth);
        }   
    }
        
    // Latency requirement
    if (latencyRequirement)
    {
        for (SDFchannelsIter iter = g->channelsBegin();
                iter != g->channelsEnd(); iter++)
        {
            c = (TimedSDFchannel*)*iter;

            randomLatencyRequirement(c, avgLatency, varLatency,
                    minLatency, maxLatency);
        }   
    }
}
