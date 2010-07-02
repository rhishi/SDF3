/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   April 3, 2006
 *
 *  Function        :   Output mapping and system usage in XML format.
 *
 *  History         :
 *      03-04-06    :   Initial version.
 *
 * $Id: xml.h,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_ARCH_XML_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_ARCH_XML_H_INCLUDED

#include "graph.h"

/**
 * createMappingNode ()
 * Create an XML node which describes the mapping of an application graph
 * onto an platform graph.
 */
CNode *createMappingNode(PlatformGraph *g, SDFgraph *appGraph);

/**
 * createPlatformGraphNode ()
 * Create an XML node which describes the platform graph.
 */
CNode *createPlatformGraphNode(PlatformGraph *g);

/**
 * createSystemUsageNode ()
 * The function constructs a system usage node in XML format.
 */
CNode *createSystemUsageNode(PlatformGraph *g);

/**
 * outputBindingAsXML ()
 * Output the binding of an SDFG to an platform graph in XML
 * format.
 */
void outputBindingAsXML(PlatformGraph *g, SDFgraph *appGraph, ostream &out);

/**
 * outputSystemUsageAsXML ()
 * Output the usage of an platform graph in XML format.
 */
void outputSystemUsageAsXML(PlatformGraph *g, ostream &out);

/**
 * outputPlatformGraphAsXML ()
 * Output the platform graph in XML format.
 */
void outputPlatformGraphAsXML(PlatformGraph *g, ostream &out);

#endif
