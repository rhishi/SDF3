/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2poosl.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert SADF Graph into POOSL Model
 *
 *  History         :
 *      13-09-06    :   Initial version.
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

#include "sadf2poosl.h"

void SADF2POOSL(SADF_Graph *Graph, SADF_SimulationSettings* Settings, CString &LogFileName, ostream &out) {

	bool ProductionOfResults = false;

	for (CId i = 0; !ProductionOfResults && i != Graph->getNumberOfKernels(); i++)
		if (Settings->getKernelSettings(i)->getMonitor() | Settings->getKernelSettings(i)->getTrace())
			ProductionOfResults = true;

	for (CId i = 0; !ProductionOfResults && i != Graph->getNumberOfDetectors(); i++)
		if (Settings->getDetectorSettings(i)->getMonitor() | Settings->getDetectorSettings(i)->getTrace())
			ProductionOfResults = true;

	for (CId i = 0; !ProductionOfResults && i != Graph->getNumberOfDataChannels(); i++)
		if (Settings->getDataChannelSettings(i)->getMonitor() | Settings->getDataChannelSettings(i)->getTrace())
			ProductionOfResults = true;

	for (CId i = 0; !ProductionOfResults && i != Graph->getNumberOfControlChannels(); i++)
		if (Settings->getControlChannelSettings(i)->getMonitor() | Settings->getControlChannelSettings(i)->getTrace())
			ProductionOfResults = true;
		
	if (!ProductionOfResults)
		throw CException("Error: No monitoring or tracing specified");

	// Start the real stuff

	out << "system specification " << Graph->getName() << endl;
	out << "behaviour specification" << endl << endl;

	SADF2POOSL_Cluster(Graph, Settings, LogFileName, out);
	
for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
	if (Graph->getKernel(i)->hasControls())
		throw CException("Error: Conversion to POOSL does not yet support multiple control inputs");

for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
	if (Graph->getDetector(i)->hasControls())
		throw CException("Error: Conversion to POOSL does not yet support multiple control inputs");

	SADF2POOSL_Process(Graph, Settings, out);
	
	SADF2POOSL_Data(out);
}
