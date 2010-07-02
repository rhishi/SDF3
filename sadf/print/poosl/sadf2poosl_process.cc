/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2poosl_process.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Output SADF graph in POOSL format (process class(es))
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

#include "sadf2poosl_process.h"

void SADF2POOSL_Kernels(SADF_Graph* Graph, ostream &out) {

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++) {

		out << "process class " << Graph->getKernel(i)->getName() << "(Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << endl;
		out << "instance variables" << endl;
		out << "Random: Uniform, Status: ProcessMonitor" << endl << endl;
		out << "initial method call" << endl;
		out << "Initialise()()" << endl << endl;
		out << "instance methods" << endl;

		out << "Initialise()()" << endl << endl;
		out << "Random := new(Uniform) withParameters(0, 1);" << endl;
		out << "Status := new(ProcessMonitor) init(Monitor, MonitorID, Trace, Name);" << endl;
		out << "abort" << endl;
		out << "   par Fire()() and if Monitor then CheckAccuracyStatus()() fi rap" << endl;
		out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi." << endl << endl;

		out << "Fire()() |Scenario: Integer|" << endl << endl;

		if (Graph->getKernel(i)->hasControlChannels()) {
			out << "Control_" << (*(Graph->getKernel(i)->getControlChannels().begin()))->getName() << "!InspectScenarioAvailability;" << endl;
			out << "Control_" << (*(Graph->getKernel(i)->getControlChannels().begin()))->getName() << "?InspectScenario(Scenario);" << endl;
		} else
			out << "Scenario := 0;" << endl;

		if (!Graph->getKernel(i)->getInputChannels().empty())
			out << "CheckTokenAvailability(Scenario)();" << endl;

		if (!Graph->getKernel(i)->getOutputChannels().empty()) {
			out << "CheckRoomAvailability(Scenario)();" << endl;
			out << "ReserveRoom(Scenario)();" << endl;
		}

		out << "Status start(currentTime);" << endl;
		
		bool Timed = false;

		for (CId j = 0; !Timed && j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
			for (CId k = 0; !Timed && k != Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles(); k++)
				if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() > 0)
					Timed = true;
		
		if (Timed)
			out << "Execute(Scenario)();" << endl;
			
		out << "Status end(currentTime);" << endl;
		
		if (!Graph->getKernel(i)->getInputChannels().empty())
			out << "ReadTokens(Scenario)();" << endl;

		if (!Graph->getKernel(i)->getOutputChannels().empty())
			out << "WriteTokens(Scenario)();" << endl;
		
		if (Graph->getKernel(i)->hasControlChannels())
			out << "Control_" << (*(Graph->getKernel(i)->getControlChannels().begin()))->getName() << "!ReadScenario;" << endl;
		
		out << "Fire()()." << endl << endl;
			
		out << "CheckAccuracyStatus()() |ID: Integer|" << endl << endl;
		out << "Monitor?Status(ID | ID = MonitorID);" << endl;
		out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << endl;
		out << "CheckAccuracyStatus()()." << endl << endl;

		if (!Graph->getKernel(i)->getInputChannels().empty()) {
		
			out << "CheckTokenAvailability(Scenario: Integer)()" << endl << endl;

			bool FirstScenario = true;

			for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {
				if (Graph->getKernel(i)->getScenario(j)->getNumberOfConsumptions() > 0) {

					if (!FirstScenario)
						out << ";" << endl;

					out << "if Scenario = " << Graph->getKernel(i)->getScenario(j)->getIdentity() << " then" << endl;
					
					for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfConsumptions(); k++) {
	
						out << "   In_" << Graph->getKernel(i)->getScenario(j)->getConsumption(k)->getChannel()->getName() << "!InspectTokenAvailability(" << Graph->getKernel(i)->getScenario(j)->getConsumption(k)->getRate() << ")";
						
						if (k < Graph->getKernel(i)->getScenario(j)->getNumberOfConsumptions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
					
					out << "fi";
					FirstScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}

		if (!Graph->getKernel(i)->getOutputChannels().empty()) {

			out << "CheckRoomAvailability(Scenario: Integer)()" << endl << endl;

			bool FirstScenario = true;

			for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {
				if (Graph->getKernel(i)->getScenario(j)->getNumberOfProductions() > 0) {

					if (!FirstScenario)
						out << ";" << endl;

					out << "if Scenario = " << Graph->getKernel(i)->getScenario(j)->getIdentity() << " then" << endl;

					for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfProductions(); k++) {
	
						out << "   Out_" << Graph->getKernel(i)->getScenario(j)->getProduction(k)->getChannel()->getName() << "!CheckRoom(" << Graph->getKernel(i)->getScenario(j)->getProduction(k)->getRate() << ")";
						
						if (k < Graph->getKernel(i)->getScenario(j)->getNumberOfProductions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
					
					out << "fi";
					FirstScenario = false;
				}
			}
			
			out << "." << endl << endl;
			
			FirstScenario = true;

			out << "ReserveRoom(Scenario: Integer)()" << endl << endl;

			for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {
				if (Graph->getKernel(i)->getScenario(j)->getNumberOfProductions() > 0) {

					if (!FirstScenario)
						out << ";" << endl;
						
					out << "if Scenario = " << Graph->getKernel(i)->getScenario(j)->getIdentity() << " then" << endl;

					for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfProductions(); k++) {
	
						out << "   Out_" << Graph->getKernel(i)->getScenario(j)->getProduction(k)->getChannel()->getName() << "!ReserveRoom";
						
						if (k < Graph->getKernel(i)->getScenario(j)->getNumberOfProductions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
				
					out << "fi";
					FirstScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}

		if (Timed) {
			out << "Execute(Scenario: Integer)() |Sample: Real|" << endl << endl;
			out << "Sample := Random sample;" << endl;

			bool FirstScenario = true;

			for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {

				bool TimedScenario = false;
				
				for (CId k = 0; !TimedScenario && k != Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles(); k++)
					if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() > 0)
						TimedScenario = true;
						
				if (TimedScenario) {
				
					if (!FirstScenario)
						out << ";" << endl;

					out << "if Scenario = " << Graph->getKernel(i)->getScenario(j)->getIdentity() << " then" << endl;

					CDouble CumulativeWeight = 0;
					bool FirstProfile = true;

					for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles(); k++) {
									
						if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() > 0) {
						
							if (!FirstProfile)
								out << ";" << endl;
							
							if (Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles() != 1) {

								out << "   if (Sample > " << CumulativeWeight << ") & (Sample <= " << CumulativeWeight + Graph->getKernel(i)->getScenario(j)->getProfile(k)->getWeight() << ") then" << endl;

								if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() != (CDouble)((unsigned long long)(Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime())))
									out << "      delay(" << Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() << ")" << endl;
								else
									out << "      delay(" << ((unsigned long long)(Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime())) << ")" << endl;

								out << "   fi";
							} else
								if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() != (CDouble)((unsigned long long)(Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime())))
									out << "  delay(" << Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() << ")" << endl;
								else
									out << "  delay(" << ((unsigned long long)(Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime())) << ")" << endl;
							
							FirstProfile = false;
						}
						
						CumulativeWeight += Graph->getKernel(i)->getScenario(j)->getProfile(k)->getWeight();
					}
					
					if (Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles() != 1)
						out << endl << "fi";
					else
						out << "fi";
					
					FirstScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}

		if (!Graph->getKernel(i)->getInputChannels().empty()) {

			out << "ReadTokens(Scenario: Integer)()" << endl << endl;

			bool FirstScenario = true;

			for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {
				if (Graph->getKernel(i)->getScenario(j)->getNumberOfConsumptions() > 0) {

					if (!FirstScenario)
						out << ";" << endl;
						
					out << "if Scenario = " << Graph->getKernel(i)->getScenario(j)->getIdentity() << " then" << endl;
					
					for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfConsumptions(); k++) {
	
						out << "   In_" << Graph->getKernel(i)->getScenario(j)->getConsumption(k)->getChannel()->getName() << "!ReadTokens";
						
						if (k < Graph->getKernel(i)->getScenario(j)->getNumberOfConsumptions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
					
					out << "fi";
					FirstScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}

		if (!Graph->getKernel(i)->getOutputChannels().empty()) {

			out << "WriteTokens(Scenario: Integer)()" << endl << endl;

			bool FirstScenario = true;

			for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++) {
				if (Graph->getKernel(i)->getScenario(j)->getNumberOfProductions() > 0) {

					if (!FirstScenario)
						out << ";" << endl;
						
					out << "if Scenario = " << Graph->getKernel(i)->getScenario(j)->getIdentity() << " then" << endl;

					for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfProductions(); k++) {
	
						out << "   Out_" << Graph->getKernel(i)->getScenario(j)->getProduction(k)->getChannel()->getName() << "!WriteTokens";
						
						if (k < Graph->getKernel(i)->getScenario(j)->getNumberOfProductions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
					
					out << "fi";
					FirstScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}
	}
}

void SADF2POOSL_Detectors(SADF_Graph* Graph, ostream &out) {

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++) {

		out << "process class " << Graph->getDetector(i)->getName() << "(Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << endl;
		out << "instance variables" << endl;
		out << "Random: Uniform, Status: ProcessMonitor, MarkovChains: Array" << endl << endl;
		out << "initial method call" << endl;
		out << "Initialise()()" << endl << endl;
		out << "instance methods" << endl;

		out << "Initialise()() |MarkovChain: MarkovChain|" << endl << endl;
		out << "Random := new(Uniform) withParameters(0, 1);" << endl;
		out << "MarkovChains := new(Array) size(" << Graph->getDetector(i)->getNumberOfScenarios() << ");" << endl;
		
		for (CId s = 0; s != Graph->getDetector(i)->getNumberOfScenarios(); s++) {
		
			out << "MarkovChain := new(MarkovChain) init;" << endl;
		
			for (CId j = 0; j != Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getNumberOfStates(); j++) {

				out << "MarkovChain addState(\"" << Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getState(j)->getName() << "\", " << Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getState(j)->getSubScenario()->getIdentity() << ");" << endl;

				CDouble CumulativeWeight = 0;
			
				for (CId k = 0; k != Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getNumberOfStates(); k++)

					if (Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getTransitionProbability(j, k) > 0) {
					
						out << "   MarkovChain addTransition(\"" << Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getState(j)->getName() << "\", \""
							<< Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getState(k)->getName() << "\", " << CumulativeWeight << ", "
							<< CumulativeWeight + Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getTransitionProbability(j, k) << ");" << endl;
				
						CumulativeWeight += Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getTransitionProbability(j, k);
					}
			}
		
			out << "MarkovChain setInitialState(\"" << Graph->getDetector(i)->getScenario(s)->getMarkovChain()->getInitialState()->getName() << "\");" << endl;
			out << "MarkovChains put(" << s + 1 << ", MarkovChain);" << endl;
		}

		out << "Status := new(ProcessMonitor) init(Monitor, MonitorID, Trace, Name);" << endl;
		out << "abort" << endl;
		out << "   par Fire()() and if Monitor then CheckAccuracyStatus()() fi rap" << endl;
		out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi." << endl << endl;

		out << "Fire()() |Scenario, SubScenario: Integer|" << endl << endl;

		if (Graph->getDetector(i)->hasControlChannels()) {
			out << "Control_" << (*(Graph->getDetector(i)->getControlChannels().begin()))->getName() << "!InspectScenarioAvailability;" << endl;
			out << "Control_" << (*(Graph->getDetector(i)->getControlChannels().begin()))->getName() << "?InspectScenario(Scenario);" << endl;
		} else
			out << "Scenario := 0;" << endl;

		out << "SubScenario := MarkovChains get(Scenario + 1) getNextSubScenario;" << endl;

		if (!Graph->getDetector(i)->getInputChannels().empty())
			out << "CheckTokenAvailability(SubScenario)();" << endl;

		if (!Graph->getDetector(i)->getOutputChannels().empty()) {
			out << "CheckRoomAvailability(SubScenario)();" << endl;
			out << "ReserveRoom(SubScenario)();" << endl;
		}

		out << "Status start(currentTime);" << endl;
		
		bool Timed = false;

		for (CId j = 0; !Timed && j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++)
			for (CId k = 0; !Timed && k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles(); k++)
				if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() > 0)
					Timed = true;
		
		if (Timed)
			out << "Execute(SubScenario)();" << endl;
			
		out << "Status end(currentTime);" << endl;
		
		if (!Graph->getDetector(i)->getInputChannels().empty())
			out << "ReadTokens(SubScenario)();" << endl;

		if (!Graph->getDetector(i)->getOutputChannels().empty())
			out << "WriteTokens(SubScenario)();" << endl;

		if (Graph->getDetector(i)->hasControlChannels())
			out << "Control_" << (*(Graph->getDetector(i)->getControlChannels().begin()))->getName() << "!ReadScenario;" << endl;
		
		out << "Fire()()." << endl << endl;
			
		out << "CheckAccuracyStatus()() |ID: Integer|" << endl << endl;
		out << "Monitor?Status(ID | ID = MonitorID);" << endl;
		out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << endl;
		out << "CheckAccuracyStatus()()." << endl << endl;

		if (!Graph->getDetector(i)->getInputChannels().empty()) {
		
			out << "CheckTokenAvailability(SubScenario: Integer)()" << endl << endl;

			bool FirstSubScenario = true;

			for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {
				if (Graph->getDetector(i)->getSubScenario(j)->getNumberOfConsumptions() > 0) {

					if (!FirstSubScenario)
						out << ";" << endl;

					out << "if SubScenario = " << Graph->getDetector(i)->getSubScenario(j)->getIdentity() << " then" << endl;
					
					for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfConsumptions(); k++) {
	
						out << "   In_" << Graph->getDetector(i)->getSubScenario(j)->getConsumption(k)->getChannel()->getName() << "!InspectTokenAvailability(" << Graph->getDetector(i)->getSubScenario(j)->getConsumption(k)->getRate() << ")";
						
						if (k < Graph->getDetector(i)->getSubScenario(j)->getNumberOfConsumptions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
					
					out << "fi";
					FirstSubScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}

		if (!Graph->getDetector(i)->getOutputChannels().empty()) {

			out << "CheckRoomAvailability(SubScenario: Integer)()" << endl << endl;

			bool FirstSubScenario = true;

			for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {
				if (Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions() > 0) {

					if (!FirstSubScenario)
						out << ";" << endl;

					out << "if SubScenario = " << Graph->getDetector(i)->getSubScenario(j)->getIdentity() << " then" << endl;

					for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions(); k++) {
	
						out << "   Out_" << Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getChannel()->getName() << "!CheckRoom(" << Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getRate() << ")";
						
						if (k < Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
					
					out << "fi";
					FirstSubScenario = false;
				}
			}
			
			out << "." << endl << endl;
			
			FirstSubScenario = true;

			out << "ReserveRoom(SubScenario: Integer)()" << endl << endl;

			for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {
				if (Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions() > 0) {

					if (!FirstSubScenario)
						out << ";" << endl;
						
					out << "if SubScenario = " << Graph->getDetector(i)->getSubScenario(j)->getIdentity() << " then" << endl;

					for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions(); k++) {
	
						out << "   Out_" << Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getChannel()->getName() << "!ReserveRoom";
						
						if (k < Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
				
					out << "fi";
					FirstSubScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}

		if (Timed) {
			out << "Execute(SubScenario: Integer)() |Sample: Real|" << endl << endl;
			out << "Sample := Random sample;" << endl;

			bool FirstSubScenario = true;

			for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {

				bool TimedSubScenario = false;
				
				for (CId k = 0; !TimedSubScenario && k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles(); k++)
					if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() > 0)
						TimedSubScenario = true;
						
				if (TimedSubScenario) {
				
					if (!FirstSubScenario)
						out << ";" << endl;

					out << "if SubScenario = " << Graph->getDetector(i)->getSubScenario(j)->getIdentity() << " then" << endl;

					CDouble CumulativeWeight = 0;
					bool FirstProfile = true;

					for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles(); k++) {
									
						if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() > 0) {
						
							if (!FirstProfile)
								out << ";" << endl;
							
							if (Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles() != 1) {

								out << "   if (Sample > " << CumulativeWeight << ") & (Sample <= " << CumulativeWeight + Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getWeight() << ") then" << endl;

								if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() != (CDouble)((unsigned long long)(Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime())))
									out << "      delay(" << Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() << ")" << endl;
								else
									out << "      delay(" << ((unsigned long long)(Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime())) << ")" << endl;

								out << "   fi";
							} else
								if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() != (CDouble)((unsigned long long)(Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime())))
									out << "  delay(" << Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() << ")" << endl;
								else
									out << "  delay(" << ((unsigned long long)(Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime())) << ")" << endl;

							FirstProfile = false;
						}
						
						CumulativeWeight += Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getWeight();
					}
					
					if (Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles() != 1)
						out << endl << "fi";
					else
						out << "fi";
					
					FirstSubScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}

		if (!Graph->getDetector(i)->getInputChannels().empty()) {

			out << "ReadTokens(SubScenario: Integer)()" << endl << endl;

			bool FirstSubScenario = true;

			for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {
				if (Graph->getDetector(i)->getSubScenario(j)->getNumberOfConsumptions() > 0) {

					if (!FirstSubScenario)
						out << ";" << endl;
						
					out << "if SubScenario = " << Graph->getDetector(i)->getSubScenario(j)->getIdentity() << " then" << endl;
					
					for (CId k= 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfConsumptions(); k++) {
	
						out << "   In_" << Graph->getDetector(i)->getSubScenario(j)->getConsumption(k)->getChannel()->getName() << "!ReadTokens";
						
						if (k < Graph->getDetector(i)->getSubScenario(j)->getNumberOfConsumptions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
					
					out << "fi";
					FirstSubScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}

		if (!Graph->getDetector(i)->getOutputChannels().empty()) {

			out << "WriteTokens(SubScenario: Integer)()" << endl << endl;

			bool FirstSubScenario = true;

			for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++) {
				if (Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions() > 0) {

					if (!FirstSubScenario)
						out << ";" << endl;
						
					out << "if SubScenario = " << Graph->getDetector(i)->getSubScenario(j)->getIdentity() << " then" << endl;

					for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions(); k++) {
	
						out << "   Out_" << Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getChannel()->getName() << "!WriteTokens";
						
						if (Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getChannel()->getType() == SADF_CONTROL_CHANNEL)
							out << "(" << Graph->getDetector(i)->getSubScenario(j)->getProduction(k)->getScenarioIdentity() << ")";
						
						if (k < Graph->getDetector(i)->getSubScenario(j)->getNumberOfProductions() - 1)
							out << ";" << endl;
						else
							out << endl;
					}
					
					out << "fi";
					FirstSubScenario = false;
				}
			}
			
			out << "." << endl << endl;
		}
	}
}

void SADF2POOSL_Process(SADF_Graph* Graph, SADF_SimulationSettings* Settings, ostream &out) {

	SADF2POOSL_Kernels(Graph, out);
	SADF2POOSL_Detectors(Graph, out);

	// Generate Process Class SimulationController

	out << "process class SimulationController(NumberOfMonitors: Integer, LogFile: String)" << endl;
	out << "instance variables" << endl;
	out << "Status: MonitorStatus" << endl << endl;
	out << "initial method call" << endl;
	out << "Initialise()()" << endl << endl;
	out << "instance methods" << endl;

	out << "Initialise()() |MonitorID: Integer, Results: String|" << endl << endl;
	out << "Status := new(MonitorStatus) init(NumberOfMonitors, LogFile);" << endl;

	if (Settings->getMaximumModelTime() != SADF_UNDEFINED) {
		if (Settings->getMaximumModelTime() != (CDouble)((unsigned long long)(Settings->getMaximumModelTime())))
			out << "abort CheckAccuracyStatus()() with delay(" << Settings->getMaximumModelTime() << ");" << endl;
		else
			out << "abort CheckAccuracyStatus()() with delay(" << ((unsigned long long)(Settings->getMaximumModelTime())) << ");" << endl;
	} else
		out << "CheckAccuracyStatus()();" << endl;

	out << "abort while true do Monitor!StopSimulation od with delay(1.0e-10);" << endl;

	out << "MonitorID := 1;" << endl;
	out << "while MonitorID <= NumberOfMonitors do" << endl;
	out << "   Monitor?Results(Results){Status append(Results)};" << endl;
	out << "   MonitorID := MonitorID + 1" << endl;
	out << "od;" << endl;
	out << "Status log." << endl << endl;

	out << "CheckAccuracyStatus()() |MonitorID: Integer, Accurate: Boolean|" << endl << endl;

	CDouble MaximumTimeStep = 0;
	CDouble MinimumTimeStep = -1;
	
	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
			for (CId k = 0; k != Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles(); k++) {
			
				if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() > MaximumTimeStep)
					MaximumTimeStep = Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime();
					
				if (MinimumTimeStep == -1)
					MinimumTimeStep = Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime();
				else			
					if (Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime() < MinimumTimeStep)
						MinimumTimeStep = Graph->getKernel(i)->getScenario(j)->getProfile(k)->getExecutionTime();
			}

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++)
			for (CId k = 0; k != Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles(); k++) {
			
				if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() > MaximumTimeStep)
					MaximumTimeStep = Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime();
					
				if (MinimumTimeStep == -1)
					MinimumTimeStep = Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime();
				else
					if (Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime() < MinimumTimeStep)
						MinimumTimeStep = Graph->getDetector(i)->getSubScenario(j)->getProfile(k)->getExecutionTime();
			}

	CDouble TimeStep = MaximumTimeStep - MinimumTimeStep;
	
	if (TimeStep != 0) {
		if (TimeStep != (CDouble)((CId)(TimeStep)))
			out << "delay(" << (5 * TimeStep) << ");" << endl;
		else
			out << "delay(" << (5 * ((CId)(TimeStep))) << ");" << endl;
	} else {
		if (MinimumTimeStep != (CDouble)((CId)(MinimumTimeStep)))
			out << "delay(" << (5 * MinimumTimeStep) << ");" << endl;
		else
			out << "delay(" << (5 * ((CId)(MinimumTimeStep))) << ");" << endl;
	}

	out << "MonitorID := 1;" << endl;
	out << "while MonitorID <= NumberOfMonitors do" << endl;
	out << "   Monitor!Status(MonitorID);" << endl;
	out << "   Monitor?AccuracyStatus(MonitorID, Accurate){Status register(MonitorID, Accurate)};" << endl;
	out << "   MonitorID := MonitorID + 1" << endl;
	out << "od;" << endl;
	out << "if (Status accurate not) then CheckAccuracyStatus()() fi." << endl << endl;

	// Generate Process Class DataBuffer
	
	out << "process class DataBuffer(BufferSize: Integer, InitialTokens: Integer, TokenSize: Integer, Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << endl;
	out << "instance variables" << endl;
	out << "Status: DataBufferMonitor" << endl << endl;
	out << "initial method call" << endl;
	out << "Initialise()()" << endl << endl;
	out << "instance methods" << endl;
	
	out << "Initialise()()" << endl << endl;
	out << "Status := new(DataBufferMonitor) init(BufferSize, InitialTokens, TokenSize, Monitor, MonitorID, Trace, Name);" << endl;
	out << "abort" << endl;
	out << "   par HandleInput()() and HandleOutput()() and if Monitor then CheckAccuracyStatus()() fi rap" << endl;
	out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi." << endl << endl;

	out << "CheckAccuracyStatus()() |ID: Integer|" << endl << endl;
	out << "Monitor?Status(ID | ID = MonitorID);" << endl;
	out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << endl;
	out << "CheckAccuracyStatus()()." << endl << endl;

	out << "HandleInput()() |NumberOfTokens: Integer|" << endl << endl;
	out << "In?CheckRoom(NumberOfTokens | Status room(NumberOfTokens));" << endl;
	out << "In?ReserveRoom{Status reserve(NumberOfTokens, currentTime)};" << endl;
	out << "In?WriteTokens{Status write(NumberOfTokens)};" << endl;
	out << "HandleInput()()." << endl << endl;
	
	out << "HandleOutput()() |NumberOfTokens: Integer|" << endl << endl;
	out << "Out?InspectTokenAvailability(NumberOfTokens | Status available(NumberOfTokens));" << endl;
	out << "Out?ReadTokens{Status remove(NumberOfTokens, currentTime)};" << endl;
	out << "HandleOutput()()." << endl << endl;

	// Generate Process Class ControlBuffer
	
	out << "process class ControlBuffer(BufferSize: Integer, NumbersInitialTokens: Queue, ContentInitialTokens: Queue, TokenSize: Integer, Monitor: Boolean, MonitorID: Integer, Trace: Boolean, Name: String)" << endl;
	out << "instance variables" << endl;
	out << "Status: ControlBufferMonitor" << endl << endl;
	out << "initial method call" << endl;
	out << "Initialise()()" << endl << endl;
	out << "instance methods" << endl;
	
	out << "Initialise()()" << endl << endl;
	out << "Status := new(ControlBufferMonitor) init(BufferSize, NumbersInitialTokens, ContentInitialTokens, TokenSize, Monitor, MonitorID, Trace, Name);" << endl;
	out << "abort" << endl;
	out << "   par HandleInput()() and HandleOutput()() and if Monitor then CheckAccuracyStatus()() fi rap" << endl;
	out << "with Monitor?StopSimulation; if Monitor then Monitor!Results(Status getResults) fi." << endl << endl;

	out << "CheckAccuracyStatus()() |ID: Integer|" << endl << endl;
	out << "Monitor?Status(ID | ID = MonitorID);" << endl;
	out << "Monitor!AccuracyStatus(MonitorID, Status accurate);" << endl;
	out << "CheckAccuracyStatus()()." << endl << endl;

	out << "HandleInput()() |NumberOfTokens: Integer, Scenario: Integer|" << endl << endl;
	out << "In?CheckRoom(NumberOfTokens | Status room(NumberOfTokens));" << endl;
	out << "In?ReserveRoom{Status reserve(NumberOfTokens, currentTime)};" << endl;
	out << "In?WriteTokens(Scenario){Status write(NumberOfTokens, Scenario)};" << endl;
	out << "HandleInput()()." << endl << endl;
	
	out << "HandleOutput()()" << endl << endl;
	out << "[Status available] Out?InspectScenarioAvailability;" << endl;
	out << "Out!InspectScenario(Status inspect);" << endl;
	out << "Out?ReadScenario{Status remove(currentTime)};" << endl;
	out << "HandleOutput()()." << endl << endl;
}


