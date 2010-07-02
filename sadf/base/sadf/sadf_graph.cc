/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_graph.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Graph
 *
 *  History         :
 *      29-08-06    :   Initial version.
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

#include "sadf_graph.h"

// Constructor

SADF_Graph::SADF_Graph(const CString &N, const CId ID) : SADF_Component(N, ID) { }

// Destructor

SADF_Graph::~SADF_Graph() {

	CId i;

	for (i = 0; i != Kernels.size(); i++)
		delete Kernels[i];

	for (i = 0; i != Detectors.size(); i++)
		delete Detectors[i];

	for (i = 0; i != DataChannels.size(); i++)
		delete DataChannels[i];

	for (i = 0; i != ControlChannels.size(); i++)
		delete ControlChannels[i];
}

// Access to instance variables

SADF_Process* SADF_Graph::getKernel(const CString &KernelName) {

	for (CId i = 0; i != Kernels.size(); i++)
		if (Kernels[i]->getName() == KernelName)
			return Kernels[i];
	
	return NULL;
}

SADF_Process* SADF_Graph::getDetector(const CString &DetectorName) {

	for (CId i = 0; i != Detectors.size(); i++)
		if (Detectors[i]->getName() == DetectorName)
			return Detectors[i];

	return NULL;
}

SADF_Channel* SADF_Graph::getChannel(const CString &ChannelName) {

	CId i;

	for (i = 0; i != DataChannels.size(); i++)
		if (DataChannels[i]->getName() == ChannelName)
			return DataChannels[i];
			
	for (i = 0; i != ControlChannels.size(); i++)
		if (ControlChannels[i]->getName() == ChannelName)
			return ControlChannels[i];
	
	return NULL;
}
