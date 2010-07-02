/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_collection.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   Transformation of strongly consistent SADF Graphs with at most one Dectector into collection of SDF Graphs
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

#include "sadf_collection.h"

bool SADF_Verify_SimpleStronglyConsistentSDFCollection(SADF_Graph* Graph, list<SDFgraph*> &Collection) {

    if (Graph->getNumberOfDetectors() == 0)
        if (SADF_Verify_SDF(Graph)) {

            SDFgraph* SDF = SADF2SDF(Graph);
            
            if (isSDFgraphConsistent(SDF)) {
                Collection.push_front(SDF);
                return true;
            }
        }

    if (Graph->getNumberOfDetectors() == 1) {
    
        bool CollectionOfSDFs = true;
    
        for (CId i = 0; CollectionOfSDFs && i != Graph->getDetector(0)->getNumberOfSubScenarios(); i++) {
        
            vector<CId> SubScenarios(1, i);
        
            SADF_Graph* ScenarioGraph = SADF_FixScenarios(Graph, SubScenarios);
            ScenarioGraph->setName(Graph->getDetector(0)->getSubScenario(i)->getName());
            
            if (SADF_Verify_SDF(ScenarioGraph)) {
            
                SDFgraph* SDF = SADF2SDF(ScenarioGraph);
                
                if (isSDFgraphConsistent(SDF)) {
                
                    RepetitionVector Vector = computeRepetitionVector(SDF);

                    if (Vector[SDF->getActor(Graph->getDetector(0)->getName())->getId()] == 1)
                        Collection.push_front(SDF);
                    else
                        CollectionOfSDFs = false;
                }
            
            } else
                CollectionOfSDFs = false;
        }
        
        if (!CollectionOfSDFs) {

            while (!Collection.empty()) {
                delete Collection.front();
                Collection.pop_front();
            }
        
        } else
            return true;
    }

    return false;
}
