/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   deadlock.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   November 13, 2007
 *
 *  Function        :   State-space based deadlock analysis
 *
 *  History         :
 *      13-11-07    :   Initial version.
 *
 * $Id: deadlock.cc,v 1.1 2008/03/06 10:49:44 sander Exp $
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

#include "deadlock.h"
#include "../../base/algo/repetition_vector.h"
#include "../../base/algo/components.h"

#define max(a,b) ((a)>(b) ? (a) : (b))
#define min(a,b) ((a)<(b) ? (a) : (b))

/******************************************************************************
 * State
 *****************************************************************************/

/**
 * printState ()
 * Print the state to the supplied stream.
 */
void SDFstateSpaceDeadlockAnalysis::TransitionSystem::State::print(
        ostream &out)
{
    out << "### State ###" << endl;

    for (uint i = 0; i < ch.size(); i++)
    {
        out << "ch[" << i << "] = " << ch[i] << endl;
    }
}

/**
 * clearState ()
 * The function sets the state to zero.
 */
void SDFstateSpaceDeadlockAnalysis::TransitionSystem::State::clear()
{
    for (uint i = 0; i < ch.size(); i++)
    {
        ch[i] = 0;
    }
}

/**
 * operator== ()
 * The function compares to states and returns true if they are equal.
 */
bool SDFstateSpaceDeadlockAnalysis::TransitionSystem::State::operator==(
    const State &s)
{
    for (uint i = 0; i < ch.size(); i++)
    {
        if (ch[i] != s.ch[i])
            return false;
    }

    return true;
}

/******************************************************************************
 * SDF
 *****************************************************************************/

#define CH(c)               currentState.ch[c]
#define CH_TOKENS(c,n)      (CH(c) >= n)  
#define CONSUME(c,n)        CH(c) = CH(c) - n;
#define PRODUCE(c,n)        CH(c) = CH(c) + n;

/**
 * actorReadyToFire ()
 * The function returns true when the actor is ready to fire in state
 * s. Else it returns false.
 */
bool SDFstateSpaceDeadlockAnalysis::TransitionSystem::actorReadyToFire(
        SDFactor *a)
{
    // Check all input ports for tokens
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        
        // Actor is destination of the channel?
        if (p->getType() == SDFport::In)
        {
            if (!CH_TOKENS(c->getId(), p->getRate()))
            {
                return false;
            }    
        }
    }

    return true;
}

/**
 * fireActor ()
 * Fire an actor. Remove tokens from all input channels and produce tokens on
 * all output channels.
 */
void SDFstateSpaceDeadlockAnalysis::TransitionSystem::fireActor(SDFactor *a)
{
    // Consume tokens from inputs and space for output tokens
    for (SDFportsIter iter = a->portsBegin(); iter != a->portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFchannel *c = p->getChannel();
        
        // Actor is destination of the channel?
        if (p->getType() == SDFport::In)
        {
            CONSUME(c->getId(), p->getRate());
        }
        else
        {
            PRODUCE(c->getId(), p->getRate());
        }
    }
}

/**  
 * execSDFgraph()  
 * Execute the SDF graph till a deadlock is found or a recurrent state.  
 * The throughput is returned.  
 */  
bool SDFstateSpaceDeadlockAnalysis::TransitionSystem::execSDFgraph()  
{
    RepetitionVector repVec;
    bool firedSomeActor;
    
    // Create initial state
    initialState.init(g->nrActors(), g->nrChannels());
    initialState.clear();  
    currentState.init(g->nrActors(), g->nrChannels());
    currentState.clear();  

    // Initial tokens and space
    for (SDFchannelsIter iter = g->channelsBegin();
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        
        CH(c->getId()) = c->getInitialTokens();
    }
    initialState = currentState;
    
    // Compute repetition vector
    repVec = computeRepetitionVector(g);
    
    // Fire the actors  
    while (true)  
    {
        firedSomeActor = false;
        
        // Fire actors when possible  
        for (SDFactorsIter iter = g->actorsBegin();
                iter != g->actorsEnd(); iter++)
        {
            TimedSDFactor *a = (TimedSDFactor*)(*iter);
        
            // Ready to fire actor a?
            while (repVec[a->getId()] != 0 && actorReadyToFire(a))
            {
                // Fire actor a
                fireActor(a);
                repVec[a->getId()]--;
                firedSomeActor = true;
            }
        }
        
        // Fired some actor in this round?
        if (!firedSomeActor)
        {
            // All actors fired as often as indicated by repVec
            for (uint i = 0; i < g->nrActors(); i++)
            {
                if (repVec[i] != 0)
                    return false;
            }
            
            // Initial state equal to current state
            if (initialState == currentState)
                return true;
                
            // Graph is not consistent and deadlock free
            return false;
        }
    }
    
    return false;
}  

/**
 * isDeadlockFree ()
 * Analyze whether an SDFG is deadlock free.
 */
bool SDFstateSpaceDeadlockAnalysis::isDeadlockFree(SDFgraph *g)
{
    // Create a transition system
    TransitionSystem transitionSystem(g);

    // Check for absense of deadlock
    return transitionSystem.execSDFgraph();
}
