/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   networkinterface.h
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
 * $Id: networkinterface.h,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#ifndef SDF_RESOURCE_ALLOCATION_MPSOC_NETWORKINTERFACE_H_INCLUDED
#define SDF_RESOURCE_ALLOCATION_MPSOC_NETWORKINTERFACE_H_INCLUDED

#include "component.h"
#include "binding.h"

/**
 * NetworkInterface
 * Container for tile network interface.
 */
class NetworkInterface : public ArchComponent
{
public:

    // Constructor
    NetworkInterface(ArchComponent &c) : ArchComponent(c)
        { connections = NULL; };
    
    // Destructor
    virtual ~NetworkInterface() { delete connections; };

    // Connections
    CSize getNrConnections() const;

    // Bandwidth
    double getInBandwidth() const;
    double getOutBandwidth() const;
    
    void setConnections(const CSize nrConnections, const double inBandwidth,
            const double outBandwidth);
    bool reserveConnection(TimedSDFchannel *c, const CSize nrConnections, 
            const double inBandwidth, const double outBandwidth);
    bool releaseConnection(TimedSDFchannel *c);
    CSize availableNrConnections() const;
    double availableInBandwidth() const;
    double availableOutBandwidth() const;

    // Bindings
    CompBindings *getBindings() const { return connections; };

    // Properties associated with binding
    enum { nrConn = 0, inBw, outBw };

private:
    // Binding
    CompBindings *connections;
};

typedef list<NetworkInterface*>             NetworkInterfaces;
typedef NetworkInterfaces::iterator         NetworkInterfacesIter;
typedef NetworkInterfaces::const_iterator   NetworkInterfacesCIter;

#endif
