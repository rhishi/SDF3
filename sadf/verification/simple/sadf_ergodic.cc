/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_ergodic.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   Ergodicity of SADF Graphs
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

#include "sadf_ergodic.h"

// Ergodicity check based on whether Markov Chains associated to detectors consist of one strongly conntected component

bool SADF_Verify_SimpleErgodicity(SADF_Graph* Graph) {

	bool Ergodic = true;

	for (CId i = 0; Ergodic && i != Graph->getNumberOfDetectors(); i++)
		for (CId j = 0; Ergodic && j != Graph->getDetector(i)->getNumberOfScenarios(); j++)
			if (!Graph->getDetector(i)->getScenario(j)->getMarkovChain()->isSingleStronglyConnectedComponent())
				Ergodic = false;
	return Ergodic;
}
