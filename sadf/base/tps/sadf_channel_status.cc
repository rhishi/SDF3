/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_channel_status.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 September 2006
 *
 *  Function        :   SADF Channel Status
 *
 *  History         :
 *      29-09-06    :   Initial version.
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

#include "sadf_channel_status.h"

// Constructors

SADF_ChannelStatus::SADF_ChannelStatus(SADF_Channel* C) {

	ReservedLocations = 0;
	AvailableTokens = C->getNumberOfInitialTokens();
}

SADF_ChannelStatus::SADF_ChannelStatus(SADF_ChannelStatus* S) {

	ReservedLocations = S->getReservedLocations();
	AvailableTokens = S->getAvailableTokens();
}

// Functions to change status

void SADF_ChannelStatus::reserve(const CId NumberOfTokens) {

	ReservedLocations += NumberOfTokens;
}

void SADF_ChannelStatus::write(const CId NumberOfTokens) {

	ReservedLocations -= NumberOfTokens;
	AvailableTokens += NumberOfTokens;
}

void SADF_ChannelStatus::remove(const CId NumberOfTokens) {

	AvailableTokens -= NumberOfTokens;
}

// Equality operator

bool SADF_ChannelStatus::equal(const SADF_ChannelStatus* S) {

	return AvailableTokens == S->getAvailableTokens();	// Equality of reserved locations follows from equality of status of writing process
}

