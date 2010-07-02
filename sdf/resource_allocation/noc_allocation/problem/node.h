/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   node.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Node
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: node.h,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_NODE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_NODE_H_INCLUDED

#include "link.h"

/**
 * Node
 * A node in the interconnect graph (router or ni).
 */
class Node
{
public:
    // Constructor
    Node(CString name, CId id) : name(name), id(id) {};
    
    // Destrcutor
    ~Node() {};

    // Id
    CId getId() const { return id; };

    // Name
    CString getName() const { return name; };
    
    // Type
    CString getType() const { return type; };
    void setType(CString t) { type = t; };
    
    // Links
    void addIncomingLink(Link *l) { incomingLinks.push_back(l); };
    void addOutgoingLink(Link *l) { outgoingLinks.push_back(l); };
    LinksIter incomingLinksBegin() { return incomingLinks.begin(); };
    LinksIter incomingLinksEnd() { return incomingLinks.end(); };
    LinksCIter incomingLinksBegin() const { return incomingLinks.begin(); };
    LinksCIter incomingLinksEnd() const { return incomingLinks.end(); };
    LinksIter outgoingLinksBegin() { return outgoingLinks.begin(); };
    LinksIter outgoingLinksEnd() { return outgoingLinks.end(); };
    LinksCIter outgoingLinksBegin() const { return outgoingLinks.begin(); };
    LinksCIter outgoingLinksEnd() const { return outgoingLinks.end(); };
    
private:
    // Name
    CString name;
    
    // Id
    CId id;
    
    // Type
    CString type;
    
    // Links
    Links incomingLinks;
    Links outgoingLinks;
};

typedef list<Node*>             Nodes;
typedef Nodes::iterator         NodesIter;
typedef Nodes::const_iterator   NodesCIter;

#endif
