/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   route.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   January 6, 2006
 *
 *  Function        :   Route
 *
 *  History         :
 *      06-01-06    :   Initial version.
 *
 * $Id: route.h,v 1.1 2008/03/20 16:16:20 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_ROUTE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_NOC_ALLOCATION_PROBLEM_ROUTE_H_INCLUDED

#include "link.h"

/**
 * Route
 * A route is a sequence of links
 */
class Route
{
public:
    // Constructor
    Route() {};
    
    // Destrcutor
    ~Route() {};

    // Links
    LinksIter linksBegin() { return links.begin(); };
    LinksIter linksEnd() { return links.end(); };
    LinksCIter linksBegin() const { return links.begin(); };
    LinksCIter linksEnd() const { return links.end(); };
    void clear() { return links.clear(); };
    uint length() const { return links.size(); };
    
    // Link addition/removal
    void insertLink(LinksIter pos, Link *l);
    void appendLink(Link *l);
    void removeLink(Link *l);
    
    // Nodes
    bool containsNode(const Node *n) const;
    
    // Cost
    void setCost(double c) { cost = c; };
    double getCost() const { return cost; };
    bool operator<(const Route &r)
            { return getCost() < r.getCost() ? true : false; };
    
    // Equality
    bool operator==(const Route &r) const;
    
    // Output
    ostream &print(ostream &out) const;
    
private:
    // Links
    Links links;
    
    // Cost
    double cost;
};

typedef list<Route>             Routes;
typedef Routes::iterator        RoutesIter;
typedef Routes::const_iterator  RoutesCIter;

#endif
