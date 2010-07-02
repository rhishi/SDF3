/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2poosl_cluster.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF graph in POOSL format (cluster class)
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

#include "sadf2poosl_cluster.h"

void SADF2POOSL_Cluster(SADF_Graph *Graph, SADF_SimulationSettings* Settings, CString &LogFileName, ostream &out) {

	CId NumberOfMonitors = 1;
	CId Counter;

	out << "(" << endl;
	
	// Instantiate Kernels
	
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {

		out << Graph->getKernel(i)->getName() << ": " << Graph->getKernel(i)->getName() << "(";
		
		if (Settings->getKernelSettings(i)->getMonitor()) {
			out << "true, " << NumberOfMonitors << ", ";
			NumberOfMonitors++;
		}
		else
			out << "false, nil, ";
			
		if (Settings->getKernelSettings(i)->getTrace())
			out << "true, \"" << Graph->getKernel(i)->getName() << "\")[";
		else
			out << "false, \"" << Graph->getKernel(i)->getName() << "\")[";

		Counter = 0;

		for (list<SADF_Channel*>::iterator j = Graph->getKernel(i)->getOutputChannels().begin(); j != Graph->getKernel(i)->getOutputChannels().end(); j++) {
			out << Graph->getKernel(i)->getName() << "_" << (*j)->getName() << "/Out_" << (*j)->getName();
			Counter++;
			if (Counter < Graph->getKernel(i)->getOutputChannels().size())
				out << ", ";
		}

		if (!Graph->getKernel(i)->getInputChannels().empty() && !Graph->getKernel(i)->getOutputChannels().empty())
			out << ", ";

		Counter = 0;

		for (list<SADF_Channel*>::iterator j = Graph->getKernel(i)->getInputChannels().begin(); j != Graph->getKernel(i)->getInputChannels().end(); j++) {
			out << (*j)->getName() << "_" << Graph->getKernel(i)->getName() << "/In_" << (*j)->getName();
			Counter++;
			if (Counter < Graph->getKernel(i)->getInputChannels().size())
				out << ", ";
		}

		Counter = 0;

		if (Graph->getKernel(i)->hasControlChannels()) {
		
			if (!Graph->getKernel(i)->getInputChannels().empty() | !Graph->getKernel(i)->getOutputChannels().empty())
				out << ", ";
			
			for (list<SADF_Channel*>::iterator j = Graph->getKernel(i)->getControlChannels().begin(); j != Graph->getKernel(i)->getControlChannels().end(); j++) {
				out << (*j)->getName() << "_" << Graph->getKernel(i)->getName() << "/Control_" << (*j)->getName();
				Counter++;
				if (Counter < Graph->getKernel(i)->getControlChannels().size())
					out << ", ";
			}
		}
		
		out << ", Monitor/Monitor]";
		
		if (i < Graph->getNumberOfKernels() - 1)
			out << " ||" << endl;
	}

	if (Graph->getNumberOfKernels() != 0 && Graph->getNumberOfDetectors() != 0)
		out << "||" << endl;

	// Instantiate Detectors

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {

		out << Graph->getDetector(i)->getName() << ": " << Graph->getDetector(i)->getName() << "(";

		if (Settings->getDetectorSettings(i)->getMonitor()) {
			out << "true, " << NumberOfMonitors << ", ";
			NumberOfMonitors++;
		}
		else
			out << "false, nil, ";
			
		if (Settings->getDetectorSettings(i)->getTrace())
			out << "true, \"" << Graph->getDetector(i)->getName() << "\")[";
		else
			out << "false, \"" << Graph->getDetector(i)->getName() << "\")[";

		Counter = 0;

		for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getOutputChannels().begin(); j != Graph->getDetector(i)->getOutputChannels().end(); j++) {
			out << Graph->getDetector(i)->getName() << "_" << (*j)->getName() << "/Out_" << (*j)->getName();
			Counter++;
			if (Counter < Graph->getDetector(i)->getOutputChannels().size())
				out << ", ";
		}

		if (!Graph->getDetector(i)->getInputChannels().empty() && !Graph->getDetector(i)->getOutputChannels().empty())
			out << ", ";

		Counter = 0;

		for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getInputChannels().begin(); j != Graph->getDetector(i)->getInputChannels().end(); j++) {
			out << (*j)->getName() << "_" << Graph->getDetector(i)->getName() << "/In_" << (*j)->getName();
			Counter++;
			if (Counter < Graph->getDetector(i)->getInputChannels().size())
				out << ", ";
		}

		Counter = 0;

		if (Graph->getDetector(i)->hasControlChannels()) {
		
			if (!Graph->getDetector(i)->getInputChannels().empty() | !Graph->getDetector(i)->getOutputChannels().empty())
				out << ", ";
			
			for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getControlChannels().begin(); j != Graph->getDetector(i)->getControlChannels().end(); j++) {
				out << (*j)->getName() << "_" << Graph->getDetector(i)->getName() << "/Control_" << (*j)->getName();
				Counter++;
				if (Counter < Graph->getDetector(i)->getControlChannels().size())
					out << ", ";
			}
		}
		
		out << ", Monitor/Monitor]";
		
		if (i < Graph->getNumberOfDetectors() - 1)
			out << " ||" << endl;
	}

	if ((Graph->getNumberOfDataChannels() != 0) | (Graph->getNumberOfControlChannels() != 0))
		out << " ||" << endl;
	
	// Instantiate Data Channels
	
	for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++) {
	
		out << Graph->getDataChannel(i)->getName() << ": DataBuffer(";
		
		if (Graph->getDataChannel(i)->getBufferSize() == SADF_UNBOUNDED)
			out << "-1, ";
		else
			out << Graph->getDataChannel(i)->getBufferSize() << ", ";
		
		out << Graph->getDataChannel(i)->getNumberOfInitialTokens() << ", " << Graph->getDataChannel(i)->getTokenSize() << ", ";
		
		if (Settings->getDataChannelSettings(i)->getMonitor()) {
			out << "true, " << NumberOfMonitors << ", ";
			NumberOfMonitors++;
		} else
			out << "false, nil, ";
			
		if (Settings->getDataChannelSettings(i)->getTrace())
			out << "true, \"" << Graph->getDataChannel(i)->getName() << "\")[";
		else
			out << "false, \"" << Graph->getDataChannel(i)->getName() << "\")[";
		
		out << Graph->getDataChannel(i)->getSource()->getName() << "_" << Graph->getDataChannel(i)->getName() << "/In, ";
		out  << Graph->getDataChannel(i)->getName() << "_" << Graph->getDataChannel(i)->getDestination()->getName() << "/Out, Monitor/Monitor]";
		
		if (i < Graph->getNumberOfDataChannels() -1 )
			out << " ||" << endl;
	}

	if (Graph->getNumberOfDataChannels() != 0 && Graph->getNumberOfControlChannels() != 0)
		out << " ||" << endl;

	// Instantiate Control Channels

	for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++) {
	
		out << Graph->getControlChannel(i)->getName() << ": ControlBuffer(";
		
		if (Graph->getControlChannel(i)->getBufferSize() == SADF_UNBOUNDED)
			out << "-1, ";
		else
			out << Graph->getControlChannel(i)->getBufferSize() << ", ";

		if (Graph->getControlChannel(i)->getNumberOfInitialTokens() == 0)
			out << "new(Queue) init, new(Queue) init, ";
		else {
			out << "new(Queue) init ";
			
			CQueue NumbersQueue(Graph->getControlChannel(i)->getNumbersQueue());
			
			while (!NumbersQueue.empty()) {
				out << "put(" << NumbersQueue.front() << ") ";
				NumbersQueue.pop();
			}
			
			out << ", new(Queue) init ";
			
			CQueue ContentQueue(Graph->getControlChannel(i)->getContentQueue());
			
			while (!ContentQueue.empty()) {
				out << "put(" << ContentQueue.front() << ") ";
				ContentQueue.pop();
			}
			
			out << ", ";
		}
		
		out << Graph->getControlChannel(i)->getTokenSize() << ", ";
		
		if (Settings->getControlChannelSettings(i)->getMonitor()) {
			out << "true, " << NumberOfMonitors << ", ";
			NumberOfMonitors++;
		} else
			out << "false, nil, ";
			
		if (Settings->getControlChannelSettings(i)->getTrace())
			out << "true, \"" << Graph->getControlChannel(i)->getName() << "\")[";
		else
			out << "false, \"" << Graph->getControlChannel(i)->getName() << "\")[";
		
		out << Graph->getControlChannel(i)->getSource()->getName() << "_" << Graph->getControlChannel(i)->getName() << "/In, ";
		out  << Graph->getControlChannel(i)->getName() << "_" << Graph->getControlChannel(i)->getDestination()->getName() << "/Out, Monitor/Monitor]";
		
		if (i < Graph->getNumberOfControlChannels() -1 )
			out << " ||" << endl;
	}

	// Instantiate Simulation Controller
	
	out << " ||" << endl;
	out << "SimulationController: SimulationController(" << NumberOfMonitors - 1 << ", \"" << LogFileName << "\")[Monitor/Monitor]" << endl;
	
	out << ")\\{";

	// Hide Data Channels

	for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++) {
		
		out << Graph->getDataChannel(i)->getSource()->getName() << "_" << Graph->getDataChannel(i)->getName() << ", ";
		out << Graph->getDataChannel(i)->getName() << "_" << Graph->getDataChannel(i)->getDestination()->getName();
		
		if (i < Graph->getNumberOfDataChannels() - 1)
			out << ", ";
	}

	if (Graph->getNumberOfDataChannels() != 0 && Graph->getNumberOfControlChannels() != 0)
		out << ", ";

	// Hide Control Channels

	for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++) {
		
		out << Graph->getControlChannel(i)->getSource()->getName() << "_" << Graph->getControlChannel(i)->getName() << ", ";
		out << Graph->getControlChannel(i)->getName() << "_" << Graph->getControlChannel(i)->getDestination()->getName();
		
		if (i < Graph->getNumberOfControlChannels() - 1)
			out << ", ";
	}

	// Hide Monitor Channel

	if ((Graph->getNumberOfDataChannels() != 0) | (Graph->getNumberOfControlChannels() != 0))
		out << ", Monitor}" << endl << endl;
	else
		out << "Monitor}" << endl << endl;
}
