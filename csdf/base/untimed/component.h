/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   component.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   CSDF base component object
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: component.h,v 1.2 2008/03/22 14:24:21 sander Exp $
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

#ifndef CSDF_BASE_COMPONENT_H_INCLUDED
#define CSDF_BASE_COMPONENT_H_INCLUDED

#include "base/base.h"
#include "../../basic_types.h"

/**
 * CSDFcomponent
 * SDF component object serves as basis for all components in an CSDF graph.
 */
class CSDFcomponent
{
public:

    // Constructor
    CSDFcomponent(CSDFcomponent *parent = NULL, const CId id = 0);
    CSDFcomponent(CSDFcomponent *parent, const CId id, const CString &name);
    
    // Desctructor
    virtual ~CSDFcomponent() {};

    // Information
    CId getId() const { return id; };
    void setId(CId i) { id = i; };
    CSDFcomponent *getParent() const { return parent; };
    CString getName() const { return name; };
    void setName(const CString &n) { name = n; };
    
private:
    // Information
    CSDFcomponent *parent;
    CId id;
    CString name;
};

#endif

