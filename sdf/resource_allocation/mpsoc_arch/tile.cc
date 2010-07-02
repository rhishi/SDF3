/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   tile.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Tile in the architecture graph.
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: tile.cc,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#include "tile.h"

/**
 * Tile ()
 * Constructor.
 */
Tile::Tile(ArchComponent &c) : ArchComponent(c)
{
    processor = NULL;
    memory = NULL;
    networkInterface = NULL;
}

/**
 * ~Tile ()
 * Destructor.
 */
Tile::~Tile()
{
    delete processor;
    delete memory;
    delete networkInterface;
}

/**
 * createProcessor ()
 * Construct a new processor and add it to the tile. A pointer to the processor
 * is returned.
 */
Processor *Tile::createProcessor(const CString &name)
{
    ArchComponent component = ArchComponent(this, 0, name);

    processor = new Processor(component);

    return processor;
}

/**
 * createMemory ()
 * Construct a new memory and add it to the tile. A pointer to the memory
 * is returned.
 */
Memory *Tile::createMemory(const CString &name)
{
    ArchComponent component = ArchComponent(this, 0, name);

    memory = new Memory(component);

    return memory;
}

/**
 * createNetworkInterface ()
 * Construct a new network interface and add it to the tile. A pointer to the 
 * network interface is returned.
 */
NetworkInterface *Tile::createNetworkInterface(const CString &name)
{
    ArchComponent component = ArchComponent(this, 0, name);

    networkInterface = new NetworkInterface(component);

    return networkInterface;
}

/**
 * addInConnection ()
 * Add an incoming connection to the tile.
 */
void Tile::addInConnection(Connection *c)
{
    inConnections.push_back(c);
}

/**
 * addOutConnection ()
 * Add an outgoing connection to the tile.
 */
void Tile::addOutConnection(Connection *c)
{
    outConnections.push_back(c);
}
