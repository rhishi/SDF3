/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2html.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF Graph in html or php Format
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

#include "sadf2html.h"

void SADF_Graph2AnnotatedDOT(SADF_Graph *Graph, bool php, ostream &out) {

	CId i;
    
	out << "digraph " << Graph->getName() << " {" << endl;
	out << "    size=\"7,10\";" << endl;

	out << endl;
    
	// Output all kernels

	for (i = 0; i != Graph->getNumberOfKernels(); i++) {
		out << "    " << Graph->getKernel(i)->getName() << " [ label=\"" << Graph->getKernel(i)->getName();
		if (php)
			out << "\", URL=\"?id=";
		else
			out << "\", URL=\"#";
		out << Graph->getKernel(i)->getName() << "\" ];" << endl;
	}

	out << endl;

	// Output all detectors

	for (i = 0; i != Graph->getNumberOfDetectors(); i++) {
		out << "    " << Graph->getDetector(i)->getName() << " [ label=\"" << Graph->getDetector(i)->getName();
		if (php)
			out << "\", style=dotted , URL=\"?id=";
		else
			out << "\", style=dotted , URL=\"#";
		out << Graph->getDetector(i)->getName() << "\" ];" << endl;
	}

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
			
			if (php)
				out << ", arrowhead=none, URL=\"?id=" << Graph->getDataChannel(i)->getName() << "\" ];" << endl;
			else
				out << ", arrowhead=none, URL=\"#" << Graph->getDataChannel(i)->getName() << "\" ];" << endl;
			
			out << "    " << Graph->getDataChannel(i)->getName();
			
			if (php)
				out << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black, URL=\"#";
			else
				out << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black, URL=\"#";
			
			out << Graph->getDataChannel(i)->getName() << "\" ];" << endl;
			out << "    " << Graph->getDataChannel(i)->getName() << " -> " << Graph->getDataChannel(i)->getDestination()->getName();
			out << " [ taillabel=\"" << Graph->getDataChannel(i)->getNumberOfInitialTokens() << "\"";
			
			if (FixedConsumptionRate)
				out << ", headlabel=\"" << ConsumptionRate << "\"";
			
			if (php)
				out << ", URL=\"?id=" << Graph->getDataChannel(i)->getName() << "\" ];" << endl;
			else
				out << ", URL=\"#" << Graph->getDataChannel(i)->getName() << "\" ];" << endl;

#else // __DOT_EXPLICIT_INITIAL_TOKENS

			out << Graph->getDataChannel(i)->getDestination()->getName() << " [ label=\"" << Graph->getDataChannel(i)->getName() << "(" << Graph->getDataChannel(i)->getNumberOfInitialTokens() << ")\"";

			if (FixedProductionRate)
				out << ", taillabel=\"" << ProductionRate << "\"";

			if (FixedConsumptionRate)
				out << ", headlabel=\"" << ConsumptionRate << "\"";

			if (php)
				out << ", URL=\"?id=" << Graph->getDataChannel(i)->getName() << "\" ];" << endl;
			else
				out << ", URL=\"#" << Graph->getDataChannel(i)->getName() << "\" ];" << endl;

#endif // __DOT_EXPLICIT_INITIAL_TOKENS

		} else {
			out << Graph->getDataChannel(i)->getDestination()->getName() << " [ label=\"" << Graph->getDataChannel(i)->getName() << "\"";

			if (FixedProductionRate)
				out << ", taillabel=\"" << ProductionRate << "\"";

			if (FixedConsumptionRate)
				out << ", headlabel=\"" << ConsumptionRate << "\"";

			if (php)
				out << ", URL=\"?id=" << Graph->getDataChannel(i)->getName() << "\" ];" << endl;
			else
				out << ", URL=\"#" << Graph->getDataChannel(i)->getName() << "\" ];" << endl;
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

			if (php)
				out << ", style=dotted, arrowhead=none, URL=\"?id=" << Graph->getControlChannel(i)->getName() << "\" ];" << endl;
			else
				out << ", style=dotted, arrowhead=none, URL=\"#" << Graph->getControlChannel(i)->getName() << "\" ];" << endl;

			out << "    " << Graph->getControlChannel(i)->getName();
			
			if (php)
				out << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black, URL=\"#";
			else
				out << " [ label=\"\", height=0.1, width=0.1, style=filled, color=black, URL=\"#";

			out << Graph->getControlChannel(i)->getName() << "\" ];" << endl;
			
			out << "    " << Graph->getDataChannel(i)->getName() << " -> " << Graph->getDataChannel(i)->getDestination()->getName();
			
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
			
			out << "\", headlabel=\"1\", style=dotted";

			if (php)
				out << ", URL=\"?id=" << Graph->getControlChannel(i)->getName() << "\" ];" << endl;
			else
				out << ", URL=\"#" << Graph->getControlChannel(i)->getName() << "\" ];" << endl;

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
				
			out << ", headlabel=\"1\", style=dotted";
			
			if (php)
				out << ", URL=\"?id=" << Graph->getControlChannel(i)->getName() << "\" ];" << endl;
			else
				out << ", URL=\"#" << Graph->getControlChannel(i)->getName() << "\" ];" << endl;
		
#endif // __DOT_EXPLICIT_INITIAL_TOKENS

		} else {
		
			out << Graph->getControlChannel(i)->getDestination()->getName() << " [ label=\"" << Graph->getControlChannel(i)->getName() << "\"";
		
			if (FixedRate)
				out << ", taillabel=\"" << Rate << "\"";
			
			if (php)
				out << ", headlabel=\"1\", style=dotted, URL=\"?id=" << Graph->getControlChannel(i)->getName() << "\" ];" << endl;
			else
				out << ", headlabel=\"1\", style=dotted, URL=\"#" << Graph->getControlChannel(i)->getName() << "\" ];" << endl;
		}
	}

	out << "}" << endl;
}

void SADF_MarkovChain2DOT(SADF_Process *Detector, CId ScenarioID, ostream &out) {

	SADF_MarkovChain *MarkovChain = Detector->getScenario(ScenarioID)->getMarkovChain();
	
	out << "digraph " << Detector->getName() << "_" << Detector->getScenario(ScenarioID)->getName() << " {" << endl;
	out << "    size=\"7,10\";" << endl;

	out << endl;
    
	// Output all states

	for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++) {
		out << "    " << MarkovChain->getState(i)->getName() << " [ label=\"" << MarkovChain->getState(i)->getName() << "\"";
		if (MarkovChain->getState(i) == MarkovChain->getInitialState())
			out << ", style=\"bold\" ];" << endl;
		else
			out << " ];" << endl;
	}

	out << endl;
	
	// Output all transitions
	
	for (CId i = 0; i != MarkovChain->getNumberOfStates(); i++)
		for (CId j = 0; j != MarkovChain->getNumberOfStates(); j++)
			if (MarkovChain->getTransitionProbability(i, j) > 0)
				out << "    " << MarkovChain->getState(i)->getName() << " -> " << MarkovChain->getState(j)->getName() << " [ label=\"" << MarkovChain->getTransitionProbability(i, j) << "\" ];" << endl;
	
	out << "}" << endl;
}

void SADF_Graph2JPG(SADF_Graph* Graph, bool php, CString &dot, CString &target, CString FileName) {

	ofstream OutputFile;

	OutputFile.open(target + FileName + ".dot");
	SADF_Graph2AnnotatedDOT(Graph, php, OutputFile);
	OutputFile.close();
	
	int exit = system(dot + "dot -Tcmapx -o" + target + FileName + ".map -Tjpg -o" + target + FileName + ".jpg " + target + FileName + ".dot");
	
	if (exit)
		throw CException((CString)("Error: Conversion from dot format to jpg format unsuccessful for SADF Graph '") + Graph->getName() + "'.");

	exit = system((CString)("rm ") + target + FileName + ".dot");

}

void SADF_MarkovChain2JPG(CString& GraphName, SADF_Process *Detector, CId ScenarioID, CString &dot, CString &target, CString FileName) {

	ofstream OutputFile;
			
	OutputFile.open(target + FileName + ".dot");
	SADF_MarkovChain2DOT(Detector, ScenarioID, OutputFile);
	OutputFile.close();

	int exit = system(dot + "dot -Tjpg -o" + target + FileName + ".jpg " + target + FileName + ".dot");

	if (exit)
		throw CException((CString)("Error: Conversion from dot format to jpg format unsuccessful for Markov Chain corresponding to Scenario '") + Detector->getScenario(ScenarioID)->getName() + "' of Detector '" +
		Detector->getName() + "' of SADF Graph '" + GraphName + "'.");
	
	exit = system((CString)("rm ") + target + FileName + ".dot");
}

void SADF2HTML(SADF_Graph *Graph, bool php, CString &dot, CString &target, CString &url, ostream &out) {

	bool FixedProductionRate = true;
	bool FixedConsumptionRate = true;
	bool FixedExecutionTime = true;
	bool AlreadyListed;
	CId ProductionRate;
	CId ConsumptionRate;
	CDouble ExecutionTime;

	ifstream InputFile;

	char c;

	// Generation of html
	
	out << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" << endl;
	out << "<html xmlns=\"http://www.w3.org/1999/xhtml\">" << endl << endl;
	out << "<head>" << endl;
	out << "  <title>Scenario-Aware Dataflow</title>" << endl;
	out << "  <meta http-equiv=\"Content-type\" content=\"text/html;charset=iso-8859-1\"/>" << endl;
	out << "  <link rel=\"stylesheet\" type=\"text/css\" media=\"screen\" href=\"http://www.es.ele.tue.nl/sadf/style/master.css\"/>" << endl;
	out << "</head>" << endl << endl;
	out << "<body>" << endl;
	
	out << "<a name=\"" << Graph->getName() << "\"></a><h1>SADF Graph: " << Graph->getName() << "</h1>" << endl << endl;

	CString GraphFileName = Graph->getName();
	SADF_Graph2JPG(Graph, php, dot, target, GraphFileName);
	
	InputFile.open(target + GraphFileName + ".map");
	c = InputFile.get();
	while (!InputFile.eof()) {
		out << c;
		c = InputFile.get();
	}
	InputFile.close();

	int exit = system((CString)("rm ") + target + GraphFileName + ".map");

	if (exit)
		throw("Error: Removing picture map failed.");

	out << "<img border=\"0\" src=\"" << url << GraphFileName << ".jpg\" usemap=\"#" << Graph->getName() << "\"/>" << endl << endl;

	if (php) {
		out << "<p>Click on an object in the image to see its properties</p>" << endl << endl;
		out << "<?php" << endl << endl << "$id = $_GET['id'];" << endl << endl;
	}

	// Output all kernel specifications

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {

		if (php)
			out << "if ($id == " << Graph->getKernel(i)->getName() << ") { ?>" << endl;		
	
		out << "<hr /><h2><a name=\"" << Graph->getKernel(i)->getName() << "\"</a>Kernel: " << Graph->getKernel(i)->getName() << "</h2>" << endl;
		
		// Print Scenarios
		
		out << "<h3>Scenarios</h3>" << endl << endl << "<p>";

		for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {
			out << Graph->getKernel(i)->getScenario(j)->getName();
			if (j < Graph->getKernel(i)->getNumberOfScenarios() - 1) out << ", ";
		}

		out << "</p>" << endl << endl;
		
		// Print Scenario Definitions
		
		if (Graph->getKernel(i)->hasControls()) {
		
			out << "<h3>Scenario Definitions</h3>" << endl << endl;

			out << "<table frame=\"hsides\" cellpadding=\"5\">" << endl;
			out << "<tr><th>Scenario</th><th>Channel</th><th>Value</th></tr>" << endl;

			for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {

				AlreadyListed = false;
				
				for (CId k = 0; k != Graph->getKernel(i)->getControlChannels().size(); k++) {
					out << "<tr>" << endl;
					if (!AlreadyListed)
						out << "  <td rowspan=\"" << Graph->getKernel(i)->getControlChannels().size() << "\">" << Graph->getKernel(i)->getScenario(j)->getName() << "</td>" << endl;
					out << "  <td>" << Graph->getControlChannel(k)->getName() << "</td>" << endl;
					out << "  <td>" << Graph->getKernel(i)->getScenario(j)->getValueReceivedFromChannel(k) << "</td>" << endl;
					out << "</tr>" << endl;
					AlreadyListed = true;
				}
			}
			
			out << "</table>" << endl << endl;
		}
		
		// Print Parameterised Rates
		
		for (list<SADF_Channel*>::iterator j = Graph->getKernel(i)->getInputChannels().begin(); j != Graph->getKernel(i)->getInputChannels().end(); j++) {
			
			ConsumptionRate = Graph->getKernel(i)->getConsumptionRate((*j)->getIdentity(), 0);
			FixedConsumptionRate = true;
		
			for (CId s = 0; FixedConsumptionRate & (s != Graph->getKernel(i)->getNumberOfScenarios()); s++)
				if (Graph->getKernel(i)->getConsumptionRate((*j)->getIdentity(), s) != ConsumptionRate)
					FixedConsumptionRate = false;
		}

		for (list<SADF_Channel*>::iterator j = Graph->getKernel(i)->getOutputChannels().begin(); j != Graph->getKernel(i)->getOutputChannels().end(); j++) {
			
			ProductionRate = Graph->getKernel(i)->getProductionRate((*j)->getIdentity(), SADF_DATA_CHANNEL, 0);
			FixedProductionRate = true;
		
			for (CId s = 0; FixedProductionRate & (s != Graph->getKernel(i)->getNumberOfScenarios()); s++)
				if (Graph->getKernel(i)->getProductionRate((*j)->getIdentity(), SADF_DATA_CHANNEL, s) != ProductionRate)
					FixedProductionRate = false;
		}
		
		if (!FixedConsumptionRate | !FixedProductionRate) {

			out << "<h3>Parameterised Rates</h3>" << endl << endl;
			out << "<table frame=\"hsides\" cellpadding=\"5\">" << endl;
			out << "<tr>" << endl;
			out << "  <th>Channel</th>" << endl;
			out << "  <th>Scenario</th>" << endl;
			out << "  <th>Rate</th>" << endl;
			out << "</tr>" << endl;
						
			for (list<SADF_Channel*>::iterator j = Graph->getKernel(i)->getInputChannels().begin(); j != Graph->getKernel(i)->getInputChannels().end(); j++) {
			
				ConsumptionRate = Graph->getKernel(i)->getConsumptionRate((*j)->getIdentity(), 0);
				FixedConsumptionRate = true;
		
				for (CId s = 0; FixedConsumptionRate & (s != Graph->getKernel(i)->getNumberOfScenarios()); s++)
					if (Graph->getKernel(i)->getConsumptionRate((*j)->getIdentity(), s) != ConsumptionRate)
						FixedConsumptionRate = false;
				
				AlreadyListed = false;
				
				if (FixedConsumptionRate == false)
					for (CId s = 0; s != Graph->getKernel(i)->getNumberOfScenarios(); s++) {
						out << "<tr>" << endl;
						if (!AlreadyListed)
 							out << "  <td rowspan=\"" << Graph->getKernel(i)->getNumberOfScenarios() << "\">" << (*j)->getName() << "</td>" << endl;
						out << "  <td>" << Graph->getKernel(i)->getScenario(s)->getName() << "</td>" << endl;
						out << "  <td>" << Graph->getKernel(i)->getConsumptionRate((*j)->getIdentity(), s) << "</td>" << endl;
						out << "</tr>" << endl;
						AlreadyListed = true;
					}
			}

			for (list<SADF_Channel*>::iterator j = Graph->getKernel(i)->getOutputChannels().begin(); j != Graph->getKernel(i)->getOutputChannels().end(); j++) {
			
				ProductionRate = Graph->getKernel(i)->getProductionRate((*j)->getIdentity(), SADF_DATA_CHANNEL, 0);
				FixedProductionRate = true;
		
				for (CId s = 0; FixedProductionRate & (s != Graph->getKernel(i)->getNumberOfScenarios()); s++)
					if (Graph->getKernel(i)->getProductionRate((*j)->getIdentity(), SADF_DATA_CHANNEL, s) != ProductionRate)
						FixedProductionRate = false;
				
				AlreadyListed = false;
				
				if (FixedProductionRate == false)
					for (CId s = 0; s != Graph->getKernel(i)->getNumberOfScenarios(); s++) {
						out << "<tr>" << endl;
						if (!AlreadyListed)
 							out << "  <td rowspan=\"" << Graph->getKernel(i)->getNumberOfScenarios() << "\">" << (*j)->getName() << "</td>" << endl;
						out << "  <td>" << Graph->getKernel(i)->getScenario(s)->getName() << "</td>" << endl;
						out << "  <td>" << Graph->getKernel(i)->getProductionRate((*j)->getIdentity(), SADF_DATA_CHANNEL, s) << "</td>" << endl;
						out << "</tr>" << endl;
						AlreadyListed = true;
					}
			}
			
			out << "</table>" << endl << endl;
		}
		
		// Print Execution Time Distributions
		
		ExecutionTime = Graph->getKernel(i)->getScenario(0)->getProfile(0)->getExecutionTime();
		FixedExecutionTime = true;
		
		for (CId j = 0; FixedExecutionTime & (j != Graph->getKernel(i)->getNumberOfScenarios()); j++)
			for (CId k = 0; FixedExecutionTime & (k != Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles()); k++)
				if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() != ExecutionTime)
					FixedExecutionTime = false;

		if (FixedExecutionTime)

			out << "<h3>Execution Time</h3>" << endl << "<p>ExecutionTime: " << ExecutionTime << "</p>" << endl << endl;

		else {
			
			out << "<h3>Execution Time Distributions</h3>" << endl << endl;
			out << "<table frame=\"hsides\" cellpadding=\"5\">" << endl;
			out << "<tr>" << endl;
			out << "  <th>Scenario</th>" << endl;
			out << "  <th>Execution Time</th>" << endl;
			out << "  <th>Probability</th>" << endl;
			out << "</tr>" << endl;
			
			for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {

				AlreadyListed = false;
	
				for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles(); k++) {
					out << "<tr>" << endl;
					if (!AlreadyListed)
						out << "  <td rowspan=\"" << Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles() <<"\">" << Graph->getKernel(i)->getScenario(j)->getName() << "</td>" << endl;
					out << "  <td>" << Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() << "</td>" << endl;
					out << "  <td>" << Graph->getKernel(i)->getScenario(j)->getProfile(k)->getWeight() << "</td>" << endl;
					out << "</tr>" << endl;
					AlreadyListed = true;
				}
			}
						
			out << "</table>" << endl << endl;

		}

		if (php)
			out << "<?php }" << endl << endl;
		else
			out << "<a href=\"#" << Graph->getName() << "\">Back to Top</a>" << endl << endl;
	}
	
	// Output all detector specifications

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {

		if (php)
			out << "if ($id == " << Graph->getDetector(i)->getName() << ") { ?>" << endl;		
	
		out << "<hr /><h2><a name=\"" << Graph->getDetector(i)->getName() << "\"</a>Detector: " << Graph->getDetector(i)->getName() << "</h2>" << endl;
		
		// Print Scenarios and SubScenarios
		
		out << "<h3>Scenarios</h3>" << endl << "<p>";

		for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++) {
			out << "<a href=\"#" << Graph->getDetector(i)->getName() << "_" << Graph->getDetector(i)->getScenario(j)->getName() << "\">" << Graph->getDetector(i)->getScenario(j)->getName() << "</a>";
			if (j < Graph->getDetector(i)->getNumberOfScenarios() - 1) out << ", ";
		}
		
		out << "</p>" << endl << endl;

		// Print Scenario Definitions
		
		if (Graph->getDetector(i)->hasControls()) {
		
			out << "<h3>Scenario Definitions</h3>" << endl << endl;

			out << "<table frame=\"hsides\" cellpadding=\"5\">" << endl;
			out << "<tr><th>Scenario</th><th>Channel</th><th>Value</th></tr>" << endl;

			for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++) {

				AlreadyListed = false;
				
				for (CId k = 0; k != Graph->getDetector(i)->getControlChannels().size(); k++) {
					out << "<tr>" << endl;
					if (!AlreadyListed)
						out << "  <td rowspan=\"" << Graph->getDetector(i)->getControlChannels().size() << "\">" << Graph->getDetector(i)->getScenario(j)->getName() << "</td>" << endl;
					out << "  <td>" << Graph->getControlChannel(k)->getName() << "</td>" << endl;
					out << "  <td>" << Graph->getDetector(i)->getScenario(j)->getValueReceivedFromChannel(k) << "</td>" << endl;
					out << "</tr>" << endl;
					AlreadyListed = true;
				}
			}
			
			out << "</table>" << endl << endl;
		}

		out << "<h3>SubScenarios</h3>" << endl << "<p>";

		for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {
			out << Graph->getDetector(i)->getSubScenario(j)->getName();
			if (j < Graph->getDetector(i)->getNumberOfSubScenarios() - 1) out << ", ";
		}
		
		out << "</p>" << endl << endl;
		
		// Print Parameterised Rates

		FixedConsumptionRate = true;
		FixedProductionRate = true;
		
		for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getInputChannels().begin(); FixedConsumptionRate && j != Graph->getDetector(i)->getInputChannels().end(); j++) {
			
			ConsumptionRate = Graph->getDetector(i)->getConsumptionRate((*j)->getIdentity(), 0);
		
			for (CId s = 0; FixedConsumptionRate & (s != Graph->getDetector(i)->getNumberOfSubScenarios()); s++)
				if (Graph->getDetector(i)->getConsumptionRate((*j)->getIdentity(), s) != ConsumptionRate)
					FixedConsumptionRate = false;
		}

		for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getOutputChannels().begin(); FixedProductionRate && j != Graph->getDetector(i)->getOutputChannels().end(); j++) {
			
			ProductionRate = Graph->getDetector(i)->getProductionRate((*j)->getIdentity(), (*j)->getType(), 0);
	
			for (CId s = 0; FixedProductionRate & (s != Graph->getDetector(i)->getNumberOfSubScenarios()); s++)
				if (Graph->getDetector(i)->getProductionRate((*j)->getIdentity(), (*j)->getType(), s) != ProductionRate)
					FixedProductionRate = false;
		}
		
		if (!FixedConsumptionRate | !FixedProductionRate) {

			out << "<h3>Parameterised Rates</h3>" << endl << endl;
			out << "<table frame=\"hsides\" cellpadding=\"5\">" << endl;
			out << "<tr>" << endl;
			out << "  <th>Channel</th>" << endl;
			out << "  <th>SubScenario</th>" << endl;
			out << "  <th>Rate</th>" << endl;
			out << "</tr>" << endl;
						
			for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getInputChannels().begin(); j != Graph->getDetector(i)->getInputChannels().end(); j++) {
			
				ConsumptionRate = Graph->getDetector(i)->getConsumptionRate((*j)->getIdentity(), 0);
				FixedConsumptionRate = true;
		
				for (CId s = 0; FixedConsumptionRate & (s != Graph->getDetector(i)->getNumberOfSubScenarios()); s++)
					if (Graph->getDetector(i)->getConsumptionRate((*j)->getIdentity(), s) != ConsumptionRate)
						FixedConsumptionRate = false;
				
				AlreadyListed = false;
				
				if (FixedConsumptionRate == false)
					for (CId s = 0; s != Graph->getDetector(i)->getNumberOfSubScenarios(); s++) {
						out << "<tr>" << endl;
						if (!AlreadyListed)
 							out << "  <td rowspan=\"" << Graph->getDetector(i)->getNumberOfSubScenarios() << "\">" << (*j)->getName() << "</td>" << endl;
						out << "  <td>" << Graph->getDetector(i)->getSubScenario(s)->getName() << "</td>" << endl;
						out << "  <td>" << Graph->getDetector(i)->getConsumptionRate((*j)->getIdentity(), s) << "</td>" << endl;
						out << "</tr>" << endl;
						AlreadyListed = true;
					}
			}

			for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getOutputChannels().begin(); j != Graph->getDetector(i)->getOutputChannels().end(); j++) {
			
				ProductionRate = Graph->getDetector(i)->getProductionRate((*j)->getIdentity(), (*j)->getType(), 0);
				FixedProductionRate = true;
		
				for (CId s = 0; FixedProductionRate & (s != Graph->getDetector(i)->getNumberOfSubScenarios()); s++)
					if (Graph->getDetector(i)->getProductionRate((*j)->getIdentity(), (*j)->getType(), s) != ProductionRate)
						FixedProductionRate = false;
				
				AlreadyListed = false;
			
				if (FixedProductionRate == false)
					for (CId s = 0; s != Graph->getDetector(i)->getNumberOfSubScenarios(); s++) {
						out << "<tr>" << endl;
						if (!AlreadyListed)
							out << "  <td rowspan=\"" << Graph->getDetector(i)->getNumberOfSubScenarios() << "\">" << (*j)->getName() << "</td>" << endl;
						out << "  <td>" << Graph->getDetector(i)->getSubScenario(s)->getName() << "</td>" << endl;
						out << "  <td>" << Graph->getDetector(i)->getProductionRate((*j)->getIdentity(), (*j)->getType(), s) << "</td>" << endl;
						out << "</tr>" << endl;
						AlreadyListed = true;
					}
			}
				
			out << "</table>" << endl << endl;
		}
		
		// Print Produced Control Tokens
		
        CId NumberOfControlOutputs = 0;
        
        for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getOutputChannels().begin(); j != Graph->getDetector(i)->getOutputChannels().end(); j++)
            if ((*j)->getType() == SADF_CONTROL_CHANNEL)
                NumberOfControlOutputs++;
                
        if (NumberOfControlOutputs > 0) {
        
    		out << "<h3>Produced Control Tokens</h3>" << endl << endl;
	    	out << "<table frame=\"hsides\" cellpadding=\"5\">" << endl;
		    out << "<tr><th>Channel</th><th>SubScenario</th><th>Value</th></tr>" << endl;
		
    		for (list<SADF_Channel*>::iterator j = Graph->getDetector(i)->getOutputChannels().begin(); j != Graph->getDetector(i)->getOutputChannels().end(); j++)
	    		if ((*j)->getType() == SADF_CONTROL_CHANNEL) {
			
		    		AlreadyListed = false;

			    	for (CId s = 0; s != Graph->getDetector(i)->getNumberOfSubScenarios(); s++) {
				    	out << "<tr>" << endl;
    					if (!AlreadyListed)
					    	out << "  <td rowspan=\"" << Graph->getDetector(i)->getNumberOfSubScenarios() << "\">" << (*j)->getName() << "</td>" << endl;
	    				out << "  <td>" << Graph->getDetector(i)->getSubScenario(s)->getName() << "</td>" << endl;
		    			out << "  <td>" << Graph->getDetector(i)->getSubScenario(s)->getValueProducedToChannel((*j)->getIdentity()) << "</td>" << endl;
			    		out << "</tr>" << endl;
				    	AlreadyListed = true;
    				}
	    		}
		
		    out << "</table>" << endl << endl;
        }
		
		// Print Execution Time Distributions
		
		ExecutionTime = Graph->getDetector(i)->getSubScenario(0)->getProfile(0)->getExecutionTime();
		FixedExecutionTime = true;
		
		for (CId j = 0; FixedExecutionTime & (j != Graph->getDetector(i)->getNumberOfSubScenarios()); j++)
			for (CId k = 0; FixedExecutionTime & (k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles()); k++)
				if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() != ExecutionTime)
					FixedExecutionTime = false;

		if (FixedExecutionTime)

			out << "<h3>Execution Time</h3>" << endl << "<p>ExecutionTime: " << ExecutionTime << "</p>" << endl << endl;

		else {
			
			out << "<h3>Execution Time Distributions</h3>" << endl << endl;
			out << "<table frame=\"hsides\" cellpadding=\"5\">" << endl;
			out << "<tr>" << endl;
			out << "  <th>SubScenario</th>" << endl;
			out << "  <th>Execution Time</th>" << endl;
			out << "  <th>Probability</th>" << endl;
			out << "</tr>" << endl;
			
			for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {

				AlreadyListed = false;
	
				for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles(); k++) {
					out << "<tr>" << endl;
					if (!AlreadyListed)
						out << "  <td rowspan=\"" << Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles() << "\">" << Graph->getDetector(i)->getSubScenario(j)->getName() << "</td>" << endl;
					out << "  <td>" << Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() << "</td>" << endl;
					out << "  <td>" << Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getWeight() << "</td>" << endl;
					out << "</tr>" << endl;
					AlreadyListed = true;
				}
			}
						
			out << "</table>" << endl << endl;	
		}
		
		// Print Markov Chains
		
		out << "<h3>Markov Chains</h3>" << endl;
		
		for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++) {

			CString MarkovChainFileName = Graph->getName() + "_" + Graph->getDetector(i)->getName() + "_" + Graph->getDetector(i)->getScenario(j)->getName();

			SADF_MarkovChain2JPG(Graph->getName(), Graph->getDetector(i), j, dot, target, MarkovChainFileName);

			out << "<table cellpadding=\"5\">" << endl;
			out << "<tr><th colspan=\"3\"><a name=\"" << Graph->getDetector(i)->getName() << "_" << Graph->getDetector(i)->getScenario(j)->getName() << "\"></a>Scenario: " << Graph->getDetector(i)->getScenario(j)->getName() << "</th></tr>";
			out << "<tr><td rowspan=\"" << Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getNumberOfStates() + 1 << "\"><img border=\"0\" src=\"" << url << MarkovChainFileName << ".jpg\"/></td>" << "<th>State</th><th>SubScenario</th></tr>" << endl;

			for (CId s = 0; s != Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getNumberOfStates(); s++) {
			
				out << "  <tr><td>" << Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getState(s)->getName() << "</td><td>" <<
				Graph->getDetector(i)->getScenario(j)->getMarkovChain()->getState(s)->getSubScenario()->getName() << "</td></tr>" << endl;
			}
			
			out << "</table>" << endl << endl;
		}
			
		if (php)
			out << "<?php }" << endl << endl;
		else
			out << "<a href=\"#" << Graph->getName() << "\">Back to Top</a>" << endl << endl;
	}

	// Output all data channels
	
	for (CId i = 0; i != Graph->getNumberOfDataChannels(); i++) {
	
		if (php)
			out << "if ($id == " << Graph->getDataChannel(i)->getName() << ") { ?>" << endl;		

		out << "<hr /><h2><a name=\"" << Graph->getDataChannel(i)->getName() << "\"></a>" << Graph->getDataChannel(i)->getName() << "</h2>" << endl;
		
		out << "<table cellpadding=\"5\">" << endl;
		
		out << "<tr><th>Buffer Size</th><th>Number of Initial Tokens</th><th>Token Size</th></tr>";
		
		if (Graph->getDataChannel(i)->getBufferSize() == SADF_UNBOUNDED)
			out << "<tr><td>Unbounded</td>";
		else
			out << "<tr><td>" << Graph->getDataChannel(i)->getBufferSize() << "</td>";
			
		out << "<td>" << Graph->getDataChannel(i)->getNumberOfInitialTokens() << "</td><td>" << Graph->getDataChannel(i)->getTokenSize() << "</td></tr>" << endl;
			
		out << "</table>" << endl;

		if (php)
			out << "<?php }" << endl << endl;
		else
			out << "<a href=\"#" << Graph->getName() << "\">Back to Top</a>" << endl << endl;
	}

	// Output all control channels
	
	for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++) {
	
		if (php)
			out << "if ($id == " << Graph->getControlChannel(i)->getName() << ") { ?>" << endl;		

		out << "<hr /><h2><a name=\"" << Graph->getControlChannel(i)->getName() << "\"></a>" << Graph->getControlChannel(i)->getName() << "</h2>" << endl;
		
		out << "<table cellpadding=\"5\">" << endl;
		
		out << "<tr><th>Buffer Size</th><th>Number of Initial Tokens</th><th>Token Size</th></tr>";
		
		if (Graph->getControlChannel(i)->getBufferSize() == SADF_UNBOUNDED)
			out << "<tr><td>Unbounded</td>";
		else
			out << "<tr><td>" << Graph->getControlChannel(i)->getBufferSize() << "</td>";
			
		out << "<td>" << Graph->getControlChannel(i)->getNumberOfInitialTokens() << "</td><td>" << Graph->getControlChannel(i)->getTokenSize() << "</td></tr>" << endl;
			
		out << "</table>" << endl;
		
		if (php)
			out << "<?php }" << endl << endl;
		else
			out << "<a href=\"#" << Graph->getName() << "\">Back to Top</a>" << endl << endl;
	}
	
	if (php)
		out << "?>" << endl;
	
	out << "</body>" << endl;
	out << "</html>" << endl;
}
