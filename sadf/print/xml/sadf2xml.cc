/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2xml.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF Graph in xml Format
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

#include "sadf2xml.h"

void SADF2XML(SADF_Graph *Graph, ostream &out) {

	// Write preamble
	
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
	out << "<sdf3 type=\"sadf\" version=\"1.0\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns=\"uri:sadf\" xsi:schemaLocation=\"uri:sadf http://www.es.ele.tue.nl/sadf/sdf3-sadf.xsd\">" << endl <<endl;
	out << "<sadf name=\"" << Graph->getName() << "\">" << endl << endl;

	// Write Structure
	
	out << "<structure>" << endl;

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		out << "  <kernel name=\"" << Graph->getKernel(i)->getName() << "\"/>" << endl;
	
	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		out << "  <detector name=\"" << Graph->getDetector(i)->getName() << "\"/>" << endl;
		
	for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
		out << "  <channel name=\"" << Graph->getDataChannel(i)->getName() << "\" type=\"data\" source=\"" << Graph->getDataChannel(i)->getSource()->getName() << "\" destination=\"" << Graph->getDataChannel(i)->getDestination()->getName() << "\"/>" << endl;
	
	for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++)
		out << "  <channel name=\"" << Graph->getControlChannel(i)->getName() << "\" type=\"control\" source=\"" << Graph->getControlChannel(i)->getSource()->getName() << "\" destination=\"" << Graph->getControlChannel(i)->getDestination()->getName() << "\"/>" << endl;
	
	out << "</structure>" << endl << endl;

	// Write Properties
	
	out << "<properties>" << endl;
	
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {
	
		out << "  <kernel_properties kernel=\"" << Graph->getKernel(i)->getName() << "\">" << endl;
		
		for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
			if (Graph->getKernel(i)->isActive(j) || Graph->getKernel(i)->hasControls() || Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles() > 1 ||
				(Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles() == 1 && Graph->getKernel(i)->getScenario(j)->getProfile(0)->getExecutionTime() > 0)) {
			
				out << "    <scenario name=\"" << Graph->getKernel(i)->getScenario(j)->getName() << "\">" << endl;
			
				for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfControls(); k++)
					out << "      <control channel=\"" << Graph->getKernel(i)->getScenario(j)->getControl(k)->getChannel()->getName() << "\" value=\"" << Graph->getKernel(i)->getScenario(j)->getControl(k)->getValue() << "\"/>" << endl;
			
				for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfConsumptions(); k++)
					out << "      <consume channel=\"" << Graph->getKernel(i)->getScenario(j)->getConsumption(k)->getChannel()->getName() << "\" tokens=\"" << Graph->getKernel(i)->getScenario(j)->getConsumption(k)->getRate() << "\"/>" << endl;
				
				for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfProductions(); k++)
					out << "      <produce channel=\"" << Graph->getKernel(i)->getScenario(j)->getProduction(k)->getChannel()->getName() << "\" tokens=\"" << Graph->getKernel(i)->getScenario(j)->getProduction(k)->getRate() << "\"/>" << endl;
				
				for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles(); k++)
					if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() != 0) {
						
						out << "      <profile execution_time=\"" << Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() << "\"";
						
						if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getWeight() < 1)
							out << " weight=\"" << Graph->getKernel(i)->getScenario(j)->getProfile(k)->getWeight() << "\"";
						
						out << "/>" << endl;
					}
		
				out << "    </scenario>" << endl;
			}
	
		out << "  </kernel_properties>" << endl << endl;
	}

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {
		
		out << "  <detector_properties detector=\"" << Graph->getDetector(i)->getName() << "\">" << endl;
		
		if (Graph->getDetector(i)->hasControls())
			for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++) {
		
				out << "    <scenario name=\"" << Graph->getDetector(i)->getScenario(j)->getName() << "\">" << endl;
				
				for (CId k = 0; k != Graph->getDetector(i)->getScenario(j)->getNumberOfControls(); k++)
					out << "      <control channel=\"" << Graph->getDetector(i)->getScenario(j)->getControl(k)->getChannel()->getName() << "\" value\"" << Graph->getDetector(i)->getScenario(j)->getControl(k)->getValue() << "\"/>" << endl;
				
				out << "      </scenario>" << endl;
			}
		
		for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {
		
			out << "    <subscenario name=\"" << Graph->getDetector(i)->getSubScenario(j)->getName() << "\">" << endl;
			
			for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfConsumptions(); k++)
				out << "      <consume channel=\"" << Graph->getDetector(i)->getSubScenario(j)->getConsumption(k)->getChannel()->getName() << "\" tokens=\"" << Graph->getDetector(i)->getSubScenario(j)->getConsumption(k)->getRate() << "\"/>" << endl;
				
			for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions(); k++) {
				
				out << "      <produce channel=\"" << Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getChannel()->getName() << "\" tokens=\"" << Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getRate() << "\"";
				
				if (Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getChannel()->getType() == SADF_CONTROL_CHANNEL)
					if (Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getValue() != Graph->getDetector(i)->getSubScenario(j)->getName())
						out << " value=\"" << Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getValue() << "\"";
					
				out << "/>" << endl;
			}
		
			for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles(); k++)
				if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() != 0) {
						
					out << "      <profile execution_time=\"" << Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() << "\"";
						
					if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getWeight() < 1)
						out << " weight=\"" << Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getWeight() << "\"";
						
					out << "/>" << endl;
				}

			out << "    </subscenario>" << endl;
		}
		
		for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++) {
		
			out << "    <markov_chain ";
			
			if (Graph->getDetector(i)->hasControlChannels())
				out << "scenario=\"" << Graph->getDetector(i)->getScenario(j)->getName() << "\" ";
			
			out << "initial_state=\"" << Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getInitialState()->getName()
			<< "\">" << endl;
			
			for (CId k = 0; k != Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getNumberOfStates(); k++) {
			
				out << "      <state name=\"" << Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getState(k)->getName() << "\" subscenario=\"" <<
				Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getState(k)->getSubScenario()->getName() << "\">" << endl;
				
				for (CId n = 0; n != Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getNumberOfStates(); n++) 
					if (Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getTransitionProbability(k, n) > 0)
						out << "        <transition destination=\"" << Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getState(n)->getName() << "\" weight=\"" <<
						Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getTransitionProbability(k, n) << "\"/>" << endl;
				
				out << "      </state>" << endl;
			}
			
			out << "    </markov_chain>" << endl;	
		}
			
		out << "  </detector_properties>" << endl << endl;	
	}

	for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++)
		if (Graph->getDataChannel(i)->getNumberOfInitialTokens() != 0 || Graph->getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED || Graph->getDataChannel(i)->getTokenSize() != 1) {

			out << "  <channel_properties channel=\"" << Graph->getDataChannel(i)->getName() << "\"";

			if (Graph->getDataChannel(i)->getNumberOfInitialTokens() != 0)
				out << " number_of_initial_tokens=\"" << Graph->getDataChannel(i)->getNumberOfInitialTokens() << "\"";

			if (Graph->getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED)
				out << " buffer_size=\"" << Graph->getDataChannel(i)->getBufferSize() << "\"" << endl;
			
			if (Graph->getDataChannel(i)->getTokenSize() != 1)
				out << " token_size=\"" << Graph->getDataChannel(i)->getTokenSize() << "\"" << endl;

			out << "/>" << endl;
		}

	for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++)
		if (Graph->getControlChannel(i)->getNumberOfInitialTokens() != 0 || Graph->getControlChannel(i)->getBufferSize() != SADF_UNBOUNDED || Graph->getControlChannel(i)->getTokenSize() != 1) {

			out << "  <channel_properties channel=\"" << Graph->getControlChannel(i)->getName() << "\"";

			if (Graph->getControlChannel(i)->getNumberOfInitialTokens() != 0) {
			
				CQueue NumbersQueue(Graph->getControlChannel(i)->getNumbersQueue());
				CQueue ContentQueue(Graph->getControlChannel(i)->getContentQueue());
				
				out << " initial_tokens=\"";
				
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
				
				out << "\"";
			}

			if (Graph->getControlChannel(i)->getBufferSize() != SADF_UNBOUNDED)
				out << " buffer_size=\"" << Graph->getControlChannel(i)->getBufferSize() << "\"" << endl;
			
			if (Graph->getControlChannel(i)->getTokenSize() != 1)
				out << " token_size=\"" << Graph->getControlChannel(i)->getTokenSize() << "\"" << endl;

			out << "/>" << endl;
		}

	out << "</properties>" << endl << endl;

	// Write postamble

	out << "</sadf>" << endl;
	out << "</sdf3>" << endl;
}
