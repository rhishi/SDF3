/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   networkinterface.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Tile network interface.
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: networkinterface.cc,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#include "networkinterface.h"

/**
 * NetworkInterface
 * Container for tile network interface.
 */
CSize NetworkInterface::getNrConnections() const
{
    return (CSize)(connections->getMaxVal(nrConn));
}

double NetworkInterface::getInBandwidth() const
{
    return connections->getMaxVal(inBw);
}

double NetworkInterface::getOutBandwidth() const
{
    return connections->getMaxVal(outBw);
}

void NetworkInterface::setConnections(const CSize nrConnections,
    const double inBandwidth, const double outBandwidth)
{
    vector<double> maxVals(3);

    // Cleanup existing connections
    delete connections;
    
    // Create new connections
    maxVals[nrConn] = nrConnections;
    maxVals[inBw] = inBandwidth;
    maxVals[outBw] = outBandwidth;
    connections = new CompBindings(maxVals);
}

bool NetworkInterface::reserveConnection(TimedSDFchannel *c,
    const CSize nrConnections, const double inBandwidth,
    const double outBandwidth)
{
    ComponentBinding *b;

    // Not enough connections available?
    if (availableNrConnections() < nrConnections)
        return false;
    
    // Not enough input bandwidth available?
    if (availableInBandwidth() < inBandwidth)
        return false;
    
    // Not enough output bandwidth available?
    if (availableOutBandwidth() < outBandwidth)
        return false;
    
    // Create component binding
    b = new ComponentBinding(c, 3);
    b->setValue(nrConn, nrConnections);
    b->setValue(inBw, inBandwidth);
    b->setValue(outBw, outBandwidth);
            
    return connections->bind(b);
}

bool NetworkInterface::releaseConnection(TimedSDFchannel *c)
{
    return connections->unbind(c);
}

CSize NetworkInterface::availableNrConnections() const
{
    double nrConnections;
    
    nrConnections = connections->getMaxVal(nrConn) 
                                    - connections->getCurVal(nrConn);
    return (CSize)(nrConnections);
    
}

double NetworkInterface::availableInBandwidth() const
{
    return connections->getMaxVal(inBw) - connections->getCurVal(inBw);
}

double NetworkInterface::availableOutBandwidth() const
{
    return connections->getMaxVal(outBw) - connections->getCurVal(outBw);
}
