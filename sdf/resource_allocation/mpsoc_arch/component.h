/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   component.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Architecture component
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: component.h,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
 *
 * $Id: component.h,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_COMPONENT_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_COMPONENT_H_INCLUDED

#include "arch_types.h"

// Forward class definition
class ArchComponent;

/**
 * ArchComponent
 * Architecture graph component object serves as basis for all components in an
 * architecture graph.
 */
class ArchComponent
{
public:

    // Constructor
    ArchComponent(ArchComponent *parent = NULL, const CId id = 0);
    ArchComponent(ArchComponent *parent, const CId id, const CString &name);
    
    // Desctructor
    virtual ~ArchComponent() {};

    // Information
    CId getId() const { return id; };
    void setId(CId i) { id = i; };
    ArchComponent *getParent() const { return parent; };
    CString getName() const { return name; };
    void setName(const CString &n) { name = n; };
    
private:
    // Information
    ArchComponent *parent;
    CId id;
    CString name;
};

#endif
