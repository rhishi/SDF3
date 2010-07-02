/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_moc.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   Subsets of SADF Graphs
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

#include "sadf_moc.h"

// Function to determine whether SADF is in fact an CSDF Graph

bool SADF_Verify_CSDF(SADF_Graph* Graph) {

    // Check cyclic occurrence of (sub)scenarios

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		for (CId j = 0; j != Graph->getDetector(i)->getNumberOfScenarios(); j++)
			if (!Graph->getDetector(i)->getScenario(j)->getMarkovChain()->isDeterministicCycle())
				return false;

    // Check profiles in (sub)scenarios

	for (CId i = 0; i != Graph->getNumberOfKernels(); i++)
		for (CId j = 0; j != Graph->getKernel(i)->getNumberOfScenarios(); j++)
			if (Graph->getKernel(i)->getScenario(j)->getNumberOfProfiles() > 1)
				return false;

	for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
		for (CId j = 0; j != Graph->getDetector(i)->getNumberOfSubScenarios(); j++)
			if (Graph->getDetector(i)->getSubScenario(j)->getNumberOfProfiles() > 1)
				return false;

    // Check initial control tokens
    
    for (CId i = 0; i != Graph->getNumberOfControlChannels(); i++)
        if (Graph->getControlChannel(i)->getNumberOfInitialTokens() > 0)
            return false;

    // Check hierarchical control

    for (CId i = 0; i != Graph->getNumberOfDetectors(); i++)
        if (Graph->getDetector(i)->hasControlChannels())
            return false;

	return true;
}

// Function to determine whether SADF Graph is in fact an SDF Graph

bool SADF_Verify_SDF(SADF_Graph* Graph) {

	bool SDF = Graph->getNumberOfDetectors() == 0;
	
	for (CId i = 0; SDF && i != Graph->getNumberOfKernels(); i++)
		if (Graph->getKernel(i)->getScenario(0)->getNumberOfProfiles() > 1)
			SDF = false;
			
	return SDF;
}

// Function to determine whether SADF Graph is in fact an HSDF Graph

bool SADF_Verify_HSDF(SADF_Graph* Graph) {

	bool HSDF = SADF_Verify_SDF(Graph);
	
	for (CId i = 0; HSDF && i != Graph->getNumberOfKernels(); i++) {
	
		for (CId j = 0; j != Graph->getKernel(i)->getScenario(0)->getNumberOfProductions(); j++)
			if (Graph->getKernel(i)->getScenario(0)->getProduction(j)->getRate() != 1)
				HSDF = false;

		for (CId j = 0; j != Graph->getKernel(i)->getScenario(0)->getNumberOfConsumptions(); j++)
			if (Graph->getKernel(i)->getScenario(0)->getConsumption(j)->getRate() != 1)
				HSDF = false;
	}
			
	return HSDF;
}

// Function to automatically detect type of MoC given SADF Graph specification

CString SADF_Determine_MoC(SADF_Graph* Graph) {

	if (SADF_Verify_HSDF(Graph))
		return "HSDF";
	else if (SADF_Verify_SDF(Graph))
		return "SDF";
	else if (SADF_Verify_CSDF(Graph))
		return "CSDF";
	else return "SADF";
}
