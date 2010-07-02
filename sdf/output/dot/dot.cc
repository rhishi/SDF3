/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   dot.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 22, 2005
 *
 *  Function        :   Output SDF graph in dot format
 *
 *  History         :
 *      22-07-05    :   Initial version.
 *
 * $Id: dot.cc,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#include "dot.h"

/**
 * outputSDFasDot ()
 * The function outputs a SDF graph in DOT format.
 */
void outputSDFasDot(SDFgraph *g, ostream &out)
{
    
    out << "digraph " << g->getName() << " {" << endl;
    out << "    size=\"7,10\";" << endl;
    
    // Output all actors
    for (SDFactorsIter iter = g->actorsBegin(); iter != g->actorsEnd(); iter++)
    {
        SDFactor *a = *iter;
        
        out << "    " << a->getName() << " [ label=\"" << a->getName();
        out << "\" ];" << endl;
    }
    out << endl;
    
    // Output all channels
    for (SDFchannelsIter iter = g->channelsBegin(); 
            iter != g->channelsEnd(); iter++)
    {
        SDFchannel *c = *iter;
        
        // Initial tokens on channel?
        if (c->getInitialTokens() != 0)
        {
#ifdef __DOT_EXPLICIT_INITIAL_TOKENS
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();
            
            // Source actor to initial token actor
            out << "    " << srcA->getName() << " -> " << c->getName();
            out << " [ label=\"" << c->getName() << "\", taillabel=\"";
            out << srcP->getRate() << "\", arrowhead=dot, headlabel=\"";
            out << c->getInitialTokens() << "\" ];" << endl;

            // initial token actor to destination actor
            out << "    " << c->getName() << " -> " << dstA->getName();
            out << " [ headlabel=\"" << dstP->getRate() << "\" ];" << endl;
            
            // set properties initial token actor
            out << "    " << c->getName();
            out << " [ label=\"\", width=0, height=0, fixedsize=true ];";
            out << endl;
#else // __DOT_EXPLICIT_INITIAL_TOKENS
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();
            
            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "(";
            out << c->getInitialTokens()<< ")" << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" ];" << endl;
#endif // __DOT_EXPLICIT_INITIAL_TOKENS
        }
        else
        {
            SDFport *srcP = c->getSrcPort();
            SDFactor *srcA = srcP->getActor();
            SDFport *dstP = c->getDstPort();
            SDFactor *dstA = dstP->getActor();
            
            out << "    " << srcA->getName() << " -> " << dstA->getName();
            out << " [ label=\"" << c->getName() << "\", taillabel=\"";
            out << srcP->getRate() << "\", headlabel=\"";
            out << dstP->getRate() << "\" ];" << endl;
        }
    }
    
    out << "}" << endl;
}
