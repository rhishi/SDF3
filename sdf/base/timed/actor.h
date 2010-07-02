/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   actor.h
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
 * $Id: actor.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_TIMED_ACTOR_H_INCLUDED
#define SDF_BASE_TIMED_ACTOR_H_INCLUDED

#include "../untimed/graph.h"
#include "timed_types.h"

/**
 * Timed SDFactor
 * Timed actor in SDF graph
 */
class TimedSDFactor : public SDFactor
{
public:
    // Processor type specific properties
    typedef struct _Processor
    {
        CString type;
        SDFtime execTime;
        CSize   stateSize;
    } Processor;

    typedef list<Processor*>            Processors;
    typedef Processors::iterator        ProcessorsIter;
    typedef Processors::const_iterator  ProcessorsCIter;

public:
    // Constructor
    TimedSDFactor(SDFcomponent &c);
    
    // Desctructor
    ~TimedSDFactor();

    // Construct
    TimedSDFactor *create(SDFcomponent &c) const;
    TimedSDFactor *createCopy(SDFcomponent &c) const;
    TimedSDFactor *clone(SDFcomponent &c) const;

    // Properties
    void setProperties(const CNodePtr propertiesNode);

    // Execution time
    SDFtime getExecutionTime();
    SDFtime getExecutionTime(const CString &proc);
    
    // State size
    CSize getStateSize();
    CSize getStateSize(const CString &proc);
    
    // Processor
    Processor *getProcessor(const CString &proc);
    Processor *addProcessor(const CString &proc);
    void addProcessor(const Processor *p);
    void removeProcessor(const CString &proc);
    uint nrProcessors() const { return processors.size(); };
    ProcessorsIter processorsBegin() { return processors.begin(); };
    ProcessorsCIter processorsBegin() const { return processors.begin(); };
    ProcessorsIter processorsEnd() { return processors.end(); };
    ProcessorsCIter processorsEnd() const { return processors.end(); };
    
    // Default processor
    CString getDefaultProcessor() const { return defaultProcessor; };
    void setDefaultProcessor(const CString &p) { defaultProcessor = p; };
    
    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, TimedSDFactor &a)
        { return a.print(out); };
    
private:
    // Processor
    Processors processors;
    CString defaultProcessor;
};

#endif
