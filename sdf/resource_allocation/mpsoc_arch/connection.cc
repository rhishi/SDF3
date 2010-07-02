/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   connection.cc
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   March 30, 2006
 *
 *  Function        :   Connection between tiles in an architecture graph
 *
 *  History         :
 *      30-03-06    :   Initial version.
 *
 * $Id: connection.cc,v 1.1.1.1 2007/10/02 10:59:47 sander Exp $
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

#include "connection.h"

/**
 * Connection ()
 * Constructor.
 */
Connection::Connection(ArchComponent &c) : ArchComponent(c)
{
    srcTile = NULL;
    dstTile = NULL;
    channelBindings = new CompBindings();
}

/**
 * ~Connection ()
 * Destructor.
 */
Connection::~Connection()
{
}

/**
 * bindChannel ()
 * The function binds a channel to the connection. On success, it
 * returns true and adss a binding of the channel to the resource. On failure,
 * it returns false and it does not change the bindings to the channel.
 */
bool Connection::bindChannel(TimedSDFchannel *c)
{
    ComponentBinding *b;

    // Create component binding
    b = new ComponentBinding(c, 0);
    return channelBindings->bind(b);
}

/**
 * unbindChannel ()
 * The function removes the binding of the channel to the connection.
 */
bool Connection::unbindChannel(TimedSDFchannel *c)
{
    return channelBindings->unbind(c);
}

