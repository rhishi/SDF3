/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   xml.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 29, 2002
 *
 *  Function        :   XML interface functions
 *
 *  History         :
 *      29-03-02    :   Initial version.
 *	    26-08-06    :   Additions by Bart Theelen.
 *
 * $Id: xml.h,v 1.3 2008/01/24 10:46:33 sander Exp $
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
 
#ifndef BASE_XML_XML_H_INCLUDED
#define BASE_XML_XML_H_INCLUDED

#include <libxml/parser.h>
#include "../string/cstring.h"

// Types
typedef xmlNode     CNode;
typedef xmlNodePtr  CNodePtr;

typedef xmlAttr     CAttr;
typedef xmlAttrPtr  CAttrPtr;

typedef xmlDoc      CDoc;
typedef xmlDocPtr   CDocPtr;

/*
 * Functions
 */
// Open a file and parse it into a tree structure
CDoc *CParseFile(const CString &filename);

// Create a new document
CDoc *CNewDoc(CNode *rootNode);

// Output a document to a file
void CSaveFile(const CString &filename, CDoc *doc, int format = 0);

// Output a document to a stream
void CSaveFile(ostream &out, CDoc *doc, int format = 0);

// Get the root node of the tree
CNode *CGetRootNode(CDoc *doc); 
 
// Return name of node
CString CIsNode(const CNode *n);

// Check that node is this node
bool CIsNode(const CNode *n, const CString &name);

// Return pointer to parent node
CNode *CGetParentNode(CNode *n);

// Get node with name name
CNode *CGetChildNode(CNode *n, const CString &name = "");

// Check that node has a child with name name  
bool CHasChildNode(CNode *n, const CString &name = "");

// Return content of node as string
CString CGetNodeContent(CNode *n);

// Add a sub-tree
CNode *CAddNode(CNode *n, const CString &name, 
                        const CString &content = "");
CNode *CAddNode(CNode *n, const CString &name, 
                        const double content); 
CNode *CAddNode(CNode *n, CNode *child);

// Create a copy of a node (including attributes and children)
CNode *CCopyNode(CNode *n);

// Create a new node
CNode *CNewNode(const CString &name);
   
// Remove node and all child nodes
void CRemoveNode(CNode *n);

// Replace node with new node (old node is returned)
CNode *CReplaceNode(CNode *oldNode, CNode *newNode);
    
// Find node with name (search in childs of node)
CNode *CFindNode(CNode *n, const CString &name);
   
// Get next node on same level (if name is not NULL,
// next node must have name 'name')
CNode *CNextNode(const CNode *n, const CString &name = "");

// Get previous node on same level (if name is not NULL,
// previous node must have name 'name')
CNode *CPreviousNode(const CNode *n, const CString &name = "");

// Get value of attribute 
CString CGetAttribute(CNode *n, const CString &name);
    
// Check that attribute is present
bool CHasAttribute(CNode *n, const CString &name);
    
// Add an attribute to the node
void CAddAttribute(CNode *n, const CString &name, const CString &value);
void CAddAttribute(CNode *n, const CString &name, const unsigned long value);

// Set or reset an attribute
void CSetAttribute(CNode *n, const CString &name, const CString &value);
   
// Remove attribute from node
void CRemoveAttribute(CNode *n, const CString &name);

// Get number of child nodes with name name
CId CGetNumberOfChildNodes(CNode *n, const CString &name);

#endif

