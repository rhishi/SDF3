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
 *  Function        :   Timed CSDF actor
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: actor.cc,v 1.2 2008/03/22 14:24:21 sander Exp $
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
 * TimedCSDFactor ()
 * Constructor.
 */
TimedCSDFactor::TimedCSDFactor(CSDFcomponent &c)
    :
        CSDFactor(c)
{
}

/**
 * ~TimedCSDFactor ()
 * Destructor.
 */
TimedCSDFactor::~TimedCSDFactor()
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
 * The function returns a pointer to a newly allocated CSDF actor object.
 */
TimedCSDFactor *TimedCSDFactor::create(CSDFcomponent &c) const
{
    return new TimedCSDFactor(c);
}

/**
 * clone ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 * The properties of this actor are also assigned to the new actor. The
 * ports are also cloned.
 */
TimedCSDFactor *TimedCSDFactor::clone(CSDFcomponent &c) const
{
    TimedCSDFactor *a = createCopy(c);
    
    // Ports
    for (CSDFportsCIter iter = portsBegin(); iter != portsEnd(); iter++)
    {
        CSDFport *p = *iter;
        CSDFcomponent component = CSDFcomponent(a, a->nrPorts());
        CSDFport *pA = p->clone(component);
        a->addPort(pA);
    }
    
    return a;
}

/**
 * createCopy ()
 * The function returns a pointer to a newly allocated CSDF actor object.
 * The properties of this actor are also assigned to the new actor.
 */
TimedCSDFactor *TimedCSDFactor::createCopy(CSDFcomponent &c) const
{
    TimedCSDFactor *a = new TimedCSDFactor(c);
    
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
 * getExecutionTime ()
 * The function returns the sequence of execution times of the actor on the
 * default processor.
 */
CSDFtimeSequence &TimedCSDFactor::getExecutionTime() 
{
    if (defaultProcessor.empty())
        throw CException("No default processor set for actor.");
    
    for (ProcessorsIter iter = processorsBegin();
            iter != processorsEnd(); iter++)
    {
        Processor *p = *iter;
        
        if (p->type == defaultProcessor)
            return p->execTime;
    }
    
    throw CException("Default processor on actor does not exist.");
}

/**
 * getStateSize ()
 * The function returns the state size of the actor on the default
 * processor.
 */
CSize TimedCSDFactor::getStateSize()
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
CSize TimedCSDFactor::getStateSize(const CString &proc)
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
TimedCSDFactor::Processor *TimedCSDFactor::getProcessor(const CString &proc)
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
TimedCSDFactor::Processor *TimedCSDFactor::addProcessor(const CString &proc)
{
    Processor *p;
   
    // Processor already in the list?
    if (getProcessor(proc) != NULL)
        throw CException("Processor already present on actor.");
 
    // Create a new processor
    p = new Processor;
    p->type = proc;
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
void TimedCSDFactor::addProcessor(const TimedCSDFactor::Processor *p)
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
void TimedCSDFactor::removeProcessor(const CString &proc)
{
    for (ProcessorsIter iter = processorsBegin();
            iter != processorsEnd();)
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
ostream &TimedCSDFactor::print(ostream &out)
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
        
        if (p->execTime.size() == 0)
        {
            out << "exec time:  (max)" << endl;
        }
        else
        {
            out << "exec time:  ";
            for (uint i = 0; i < p->execTime.size(); i++)
            {
                if (i != 0)
                    out << ", ";
                out << p->execTime[i];
            }
            out << endl;
        }
        
        if (p->stateSize == CSIZE_MAX)
            out << "state size: (max)" << endl;
        else
            out << "state size: " << p->stateSize << endl;
    }
    
    out << endl;

    CSDFports &ports = getPorts();
    for (CSDFportsIter iter = ports.begin(); iter != ports.end(); iter++)
    {
        CSDFport *p = *iter;
        
        p->print(out);
    }
    
    return out;
}
