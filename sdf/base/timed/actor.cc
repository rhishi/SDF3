/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 18, 2005
 *
 *  Function        :   Timed SDF actor
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: actor.cc,v 1.2 2008/05/15 09:18:17 sander Exp $
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

#include "actor.h"

/**
 * TimedSDFactor ()
 * Constructor.
 */
TimedSDFactor::TimedSDFactor(SDFcomponent &c)
    :
        SDFactor(c)
{
}

/**
 * ~TimedSDFactor ()
 * Destructor.
 */
TimedSDFactor::~TimedSDFactor()
{
    // Processor
    for (ProcessorsIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        Processor *p = *iter;
        
        delete p;
    }
}

/**
 * create ()
 * The function returns a pointer to a newly allocated SDF actor object.
 */
TimedSDFactor *TimedSDFactor::create(SDFcomponent &c) const
{
    return new TimedSDFactor(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated SDF actor object.
 * The properties of this actor are also assigned to the new actor. The
 * ports are also cloned.
 */
TimedSDFactor *TimedSDFactor::clone(SDFcomponent &c) const
{
    TimedSDFactor *a = createCopy(c);
    
    // Ports
    for (SDFportsCIter iter = portsBegin(); iter != portsEnd(); iter++)
    {
        SDFport *p = *iter;
        SDFcomponent component = SDFcomponent(a, a->nrPorts());
        SDFport *pA = p->clone(component);
        a->addPort(pA);
    }
    
    return a;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated SDF actor object.
 * The properties of this actor are also assigned to the new actor.
 */
TimedSDFactor *TimedSDFactor::createCopy(SDFcomponent &c) const
{
    TimedSDFactor *a = new TimedSDFactor(c);
    
    // Properties
    a->setName(getName());
    a->setType(getType());
    a->setDefaultProcessor(getDefaultProcessor());
    
    // Processors
    for (ProcessorsCIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        a->addProcessor(*iter);
    }
    
    return a;
}

/**
 * setProperties ()
 * Set the properties of a timed actor.
 */
void TimedSDFactor::setProperties(const CNodePtr propertiesNode)
{
    TimedSDFactor::Processor *p;
    
    // Processor definitions
    for (CNode *procNode = CGetChildNode(propertiesNode, "processor"); 
            procNode != NULL; procNode = CNextNode(procNode, "processor"))
    {
        if (!CHasAttribute(procNode, "type"))
            throw CException("Processor must have a type");
    
        // Add a processor to the actor
        p = addProcessor(CGetAttribute(procNode, "type"));
        
        // Execution time
        if (CHasChildNode(procNode, "executionTime"))
        {
            CNode *execTimeNode = CGetChildNode(procNode, "executionTime");

            if (!CHasAttribute(execTimeNode, "time"))
                throw CException("Execution time not specified");

            p->execTime = CGetAttribute(execTimeNode, "time");
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
            setDefaultProcessor(p->type);
        }
    }
}

/**
 * getExecutionTime ()
 * The function returns the execution time of the actor on the default
 * processor.
 */
SDFtime TimedSDFactor::getExecutionTime() 
{
    if (defaultProcessor.empty())
        return 0;
    
    for (ProcessorsIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        Processor *p = *iter;
        
        if (p->type == defaultProcessor)
            return p->execTime;
    }
    
    return SDFTIME_MAX;
}

/**
 * getExecutionTime ()
 * The function returns the execution time of the actor on the processor.
 */
SDFtime TimedSDFactor::getExecutionTime(const CString &proc)
{
    for (ProcessorsIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        Processor *p = *iter;
        
        if (p->type == proc)
            return p->execTime;
    }
    
    return SDFTIME_MAX;
}

/**
 * getStateSize ()
 * The function returns the state size of the actor on the default
 * processor.
 */
CSize TimedSDFactor::getStateSize()
{
    if (defaultProcessor.empty())
        return CSIZE_MAX;
    
    for (ProcessorsIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        Processor *p = *iter;
        
        if (p->type == defaultProcessor)
            return p->stateSize;
    }
    
    return CSIZE_MAX;
}

/**
 * getStateSize ()
 * The function returns the state size of the actor on the processor.
 */
CSize TimedSDFactor::getStateSize(const CString &proc)
{
    for (ProcessorsIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        Processor *p = *iter;
        
        if (p->type == proc)
            return p->stateSize;
    }
    
    return CSIZE_MAX;
}

/**
 * getProcessor ()
 * Get properties of actor for specified processor type
 */
TimedSDFactor::Processor *TimedSDFactor::getProcessor(const CString &proc)
{
    for (ProcessorsCIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        Processor *p = *iter;
        
        if (p->type == proc)
            return p;
    }
    
    return NULL;
}

/**
 * addProcessor ()
 * Add a processor type to the list of processor types the actor can be mapped
 * to. A reference to the properties container for the processor is returned.
 */
TimedSDFactor::Processor *TimedSDFactor::addProcessor(const CString &proc)
{
    Processor *p;
   
    // Processor already in the list?
    if (getProcessor(proc) != NULL)
        throw CException("Processor already present on actor.");
 
    // Create a new processor
    p = new Processor;
    p->type = proc;
    p->execTime = SDFTIME_MAX;
    p->stateSize = CSIZE_MAX;
    
    // Add p to list of processors supported by the actor
    processors.push_back(p);
    
    return p;
}

/**
 * addProcessor ()
 * Add a processor type to the list of processor types the actor can be mapped
 * to. A reference to the properties container for the processor is returned.
 */
void TimedSDFactor::addProcessor(const TimedSDFactor::Processor *p)
{
    Processor *pa;
 
    // Processor already in the list?
    if (getProcessor(p->type) != NULL)
        throw CException("Processor already present on actor.");
    
    // Create a new processor
    pa = new Processor;
    pa->type = p->type;
    pa->execTime = p->execTime;
    pa->stateSize = p->stateSize;
    
    // Add pa to list of processors supported by the actor
    processors.push_back(pa);
}

/**
 * removeProcessor ()
 * Remove a processor type from the list of processor types the actor can be
 * mapped to.
 */
void TimedSDFactor::removeProcessor(const CString &proc)
{
    for (ProcessorsIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        Processor *p = *iter;
    
        if (p->type == proc)
        {
            delete p;
            processors.erase(iter);
            return;
        }
    }    
}

/**
 * print ()
 * Print the actor to the supplied output stream.
 */
ostream &TimedSDFactor::print(ostream &out)
{
    out << "Actor (" << getName() << ")" << endl;
    out << "id:         " << getId() << endl;
    out << "type:       " << getType() << endl;
    
    for (ProcessorsIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        Processor *p = *iter;
        
        out << "processor:  " << p->type;
        if (p->type == defaultProcessor)
            out << " (default)";
        out << endl;
        
        if (p->execTime == SDFTIME_MAX)
            out << "exec time:  (max)" << endl;
        else
            out << "exec time:  " << p->execTime << endl;
    
        if (p->stateSize == CSIZE_MAX)
            out << "state size: (max)" << endl;
        else
            out << "state size: " << p->stateSize << endl;
    }
    
    out << endl;

    for (SDFportsIter iter = portsBegin(); iter != portsEnd(); iter++)
    {
        SDFport *p = *iter;
        
        p->print(out);
    }
    
    return out;
}
