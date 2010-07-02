/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2dot.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF Graph in DOT Format
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

#include "sadf2dot.h"

void SADF2DOT(SADF_Graph *Graph, ostream &out) {

	CId i;
    
	out << "digraph " << Graph->getName() << " {" << endl;
	out << "    size=\"7,10\";" << endl;

	out << endl;
    
	// Output all kernels

	for (i = 0; i != Graph->getNumberOfKernels(); i++)
		out << "    " << Graph->getKernel(i)->getName() << " [ label=\"" << Graph->getKernel(i)->getName() << "\" ];" << endl;

	out << endl;

	// Output all detectors

	for (i = 0; i != Graph->getNumberOfDetectors(); i++)
		out << "    " << Graph->getDetector(i)->getName() << " [ label=\"" << Graph->getDetector(i)->getName() << "\", style=dotted ];" << endl;

	out << endl;

	// Output all data channels
    
	for (i = 0; i != Graph->getNumberOfDataChannels(); i++) {

		bool FixedProductionRate = true;
		bool FixedConsumptionRate = true;
		CId ProductionRate = 0;
		CId ConsumptionRate = 0;

		SADF_Process* Process = Graph->getDataChannel(i)->getSource();

		if (Process->getType() == SADF_KERNEL)
			for (CId n = 0; FixedProductionRate && n != Process->getNumberOfScenarios(); n++) {
				if (Process->getProductionRate(i, SADF_DATA_CHANNEL, n) == 0)
					FixedProductionRate = false;
				else
					for (CId m = 0; m != Process->getScenario(n)->getNumberOfProductions(); m++)
						if (Process->getScenario(n)->getProduction(m)->getChannel()->getIdentity() == i) {
							if (ProductionRate == 0)
								ProductionRate = Process->getScenario(n)->getProduction(m)->getRate();
							else
								if (Process->getScenario(n)->getProduction(m)->getRate() != ProductionRate)
									FixedProductionRate = false;
						}
			}

		if (Process->getType() == SADF_DETECTOR)
			for (CId n = 0; FixedProductionRate && n != Process->getNumberOfSubScenarios(); n++) {
				if (Process->getProductionRate(i, SADF_DATA_CHANNEL, n) == 0)
					FixedProductionRate = false;
				else
					for (CId m = 0; m != Process->getSubScenario(n)->getNumberOfProductions(); m++)
						if (Process->getSubScenario(n)->getProduction(m)->getChannel()->getIdentity() == i) {
							if (ProductionRate == 0)
								ProductionRate = Process->getSubScenario(n)->getProduction(m)->getRate();
							else
								if (Process->getSubScenario(n)->getProduction(m)->getRate() != ProductionRate)
									FixedProductionRate = false;
						}
			}

		Process = Graph->getDataChannel(i)->getDestination();

		if (Process->getType() == SADF_KERNEL)
			for (CId n = 0; FixedConsumptionRate && n != Process->getNumberOfScenarios(); n++) {
				if (Process->getConsumptionRate(i, n) == 0)
					FixedConsumptionRate = false;
				else
					for (CId m = 0; m != Process->getScenario(n)->getNumberOfConsumptions(); m++)
						if (Process->getScenario(n)->getConsumption(m)->getChannel()->getIdentity() == i) {
							if (ConsumptionRate == 0)
								ConsumptionRate = Process->getScenario(n)->getConsumption(m)->getRate();
							else
								if (Process->getScenario(n)->getConsumption(m)->getRate() != ConsumptionRate)
									FixedConsumptionRate = false;
						}
			}

		if (Process->getType() == SADF_DETECTOR)
			for (CId n = 0; FixedConsumptionRate && n != Process->getNumberOfSubScenarios(); n++) {
				if (Process->getConsumptionRate(i, n) == 0)
					FixedConsumptionRate = false;
				else
					for (CId m = 0; m != Process->getSubScenario(n)->getNumberOfConsumptions(); m++)
						if (Process->getSubScenario(n)->getConsumption(m)->getChannel()->getIdentity() == i) {
							if (ConsumptionRate == 0)
								ConsumptionRate = Process->getSubScenario(n)->getConsumption(m)->getRate();
							else
								if (Process->getSubScenario(n)->getConsumption(m)->getRate() != ConsumptionRate)
									FixedConsumptionRate = false;
						}
			}

		out << "    " << Graph->getDataChannel(i)->getSource()->getName() << " -> ";

		if (Graph->getDataChannel(i)->getNumberOfInitialTokens() != 0) {
			
#ifdef __DOT_EXPLICIT_INITIAL_TOKENS

			out << Graph->getDataChannel(i)->getName() << " [ label=\"" << Graph->getDataChannel(i)->getName() << "\"";
			
			if (FixedProductionRate)
				out << ", taillabel=\"" << ProductionRate << "\"";
			
			out << ", arrowhead=none ];" << endl;
			
			out << "    " << Graph->getDataChannel(i)->getName() << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black ];" << endl;

			out << "    " << Graph->getDataChannel(i)->getName() << " -> " << Graph->getDataChannel(i)->getDestination()->getName();

			out << " [ taillabel=\"" << Graph->getDataChannel(i)->getNumberOfInitialTokens() << "\"";
			
			if (FixedConsumptionRate)
				out << ", headlabel=\"" << ConsumptionRate << "\"";
			
			out << " ];" << endl;

#else // __DOT_EXPLICIT_INITIAL_TOKENS

			out << Graph->getDataChannel(i)->getDestination()->getName() << " [ label=\"" << Graph->getDataChannel(i)->getName() << "(" << Graph->getDataChannel(i)->getNumberOfInitialTokens() << ")\"";

			if (FixedProductionRate)
				out << ", taillabel=\"" << ProductionRate << "\"";

			if (FixedConsumptionRate)
				out << ", headlabel=\"" << ConsumptionRate << "\"";

			out << " ];" << endl;

#endif // __DOT_EXPLICIT_INITIAL_TOKENS

		} else {
			out << Graph->getDataChannel(i)->getDestination()->getName() << " [ label=\"" << Graph->getDataChannel(i)->getName() << "\"";

			if (FixedProductionRate)
				out << ", taillabel=\"" << ProductionRate << "\"";

			if (FixedConsumptionRate)
				out << ", headlabel=\"" << ConsumptionRate << "\"";

			out << " ];" << endl;
		}
	}

	out << endl;

	// Output all control channels
    
	for (i = 0; i != Graph->getNumberOfControlChannels(); i++) {

		SADF_Process* Detector = Graph->getControlChannel(i)->getSource();

		bool FixedRate = true;
		CId Rate = 0;

		for (CId n = 0; FixedRate && n != Detector->getNumberOfSubScenarios(); n++)
			for (CId m = 0; m != Detector->getSubScenario(n)->getNumberOfProductions(); m++)
				if (Detector->getSubScenario(n)->getProduction(m)->getChannel()->getIdentity() == i) {
					if (Rate == 0)
						Rate = Detector->getSubScenario(n)->getProduction(m)->getRate();
					else
						if (Detector->getSubScenario(n)->getProduction(m)->getRate() != Rate)
							FixedRate = false;
				}

		out << "    " << Graph->getControlChannel(i)->getSource()->getName() << " -> ";
		
		if (Graph->getControlChannel(i)->getNumberOfInitialTokens() != 0) {

#ifdef __DOT_EXPLICIT_INITIAL_TOKENS
		
			out << Graph->getControlChannel(i)->getName() << " [ label=\"" << Graph->getControlChannel(i)->getName() << "\"";
			
			if (FixedRate)
				out << ", taillabel=\"" << Rate << "\"";
			
			out << ", style=dotted, arrowhead=none ];" << endl;
			
			out << "    " << Graph->getControlChannel(i)->getName() << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black ];" << endl;

			out << "    " << Graph->getControlChannel(i)->getName() << " -> " << Graph->getControlChannel(i)->getDestination()->getName();

			out << " [ taillabel=\"";

			CQueue NumbersQueue(Graph->getControlChannel(i)->getNumbersQueue());
			CQueue ContentQueue(Graph->getControlChannel(i)->getContentQueue());
			
			while (!NumbersQueue.empty()) {
			
				CId Scenario = ContentQueue.front();
				CString ScenarioName = "";
				
				SADF_Process* Destination = Graph->getControlChannel(i)->getDestination();
					
				if (!Destination->hasControls())
					ScenarioName = Destination->getScenario(Scenario)->getName();
				else {
					bool ScenarioNotFound = true;
						
					for (CId j = 0; ScenarioNotFound && j != Destination->getNumberOfScenarios(); j++)
						if (Destination->getScenario(j)->getScenarioIdentityReceivedFromChannel(i) == Scenario) {
							ScenarioNotFound = false;
							ScenarioName = Destination->getScenario(j)->getValueReceivedFromChannel(i);
						}
				}

				if (NumbersQueue.front() == 1)
					out << ScenarioName;
				else
					out << NumbersQueue.front() << "*" << ScenarioName;
					
				if (NumbersQueue.size() != 1)
					out << ", ";
			
				NumbersQueue.pop();
				ContentQueue.pop();
			}
			
			out << "\", headlabel=\"1\", style=dotted ];" << endl;
		
#else // __DOT_EXPLICIT_INITIAL_TOKENS

			out << Graph->getControlChannel(i)->getDestination()->getName() << " [ label=\"" << Graph->getControlChannel(i)->getName() << "(";
			
			CQueue NumbersQueue(Graph->getControlChannel(i)->getNumbersQueue());
			CQueue ContentQueue(Graph->getControlChannel(i)->getContentQueue());
			
			while (!NumbersQueue.empty()) {
			
				CId Scenario = ContentQueue.front();
				CString ScenarioName = "";
				
				SADF_Process* Destination = Graph->getControlChannel(i)->getDestination();
					
				if (!Destination->hasControls())
					ScenarioName = Destination->getScenario(Scenario)->getName();
				else {
					bool ScenarioNotFound = true;
						
					for (CId j = 0; ScenarioNotFound && j != Destination->getNumberOfScenarios(); j++)
						if (Destination->getScenario(j)->getScenarioIdentityReceivedFromChannel(i) == Scenario) {
							ScenarioNotFound = false;
							ScenarioName = Destination->getScenario(j)->getValueReceivedFromChannel(i);
						}
				}

				if (NumbersQueue.front() == 1)
					out << ScenarioName;
				else
					out << NumbersQueue.front() << "*" << ScenarioName;
					
				if (NumbersQueue.size() != 1)
					out << ", ";
			
				NumbersQueue.pop();
				ContentQueue.pop();
			}
			
			out << ")\"";
			
			if (FixedRate)
				out << ", taillabel=\"" << Rate << "\"";
				
			out << ", headlabel=\"1\", style=dotted ];" << endl;
		
#endif // __DOT_EXPLICIT_INITIAL_TOKENS
		
		} else {
		
			out << Graph->getControlChannel(i)->getDestination()->getName() << " [ label=\"" << Graph->getControlChannel(i)->getName() << "\"";
		
			if (FixedRate)
				out << ", taillabel=\"" << Rate << "\"";
			
			out << ", headlabel=\"1\", style=dotted ];" << endl;
		}
	}

	out << "}" << endl;
}
