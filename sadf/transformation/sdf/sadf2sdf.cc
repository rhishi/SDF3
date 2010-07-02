/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf2sdf.cc
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   13 September 2006
 *
 *  Function        :   Convert SADF Graph in SDF Graph
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

#include "sadf2sdf.h"

TimedSDFgraph* SADF2SDF(SADF_Graph *SADF) {

	// Check whether conversion is possible

	if (!SADF_Verify_SDF(SADF))
		throw CException((CString)("Error: SADF Graph '") + SADF->getName() + "' does not represent an SDF Graph.");
		
	for (CId i = 0; i != SADF->getNumberOfKernels(); i++) {
		if (SADF->getKernel(i)->getScenario(0)->getProfile(0)->getExecutionTime() != (CDouble)((unsigned long long)(SADF->getKernel(i)->getScenario(0)->getProfile(0)->getExecutionTime())))
			throw CException((CString)("Error: Real-valued execution time for Kernel '") + SADF->getKernel(i)->getName() + "' not surported.");

		if (SADF->getKernel(i)->getScenario(0)->getProfile(0)->getExecutionTime() != (CDouble)((SDFtime)(SADF->getKernel(i)->getScenario(0)->getProfile(0)->getExecutionTime())))
			throw CException((CString)("Error: Execution time for Kernel '") + SADF->getKernel(i)->getName() + "' is too large to be supported.");
	}

	// Create SDF graph	

	SDFcomponent Component = new SDFcomponent(NULL, SADF->getIdentity());
	TimedSDFgraph* SDF = new TimedSDFgraph(Component);
	SDF->setName(SADF->getName());
	SDF->setType("sdf");
	
	// Create Actors
	
	for (CId i = 0; i != SADF->getNumberOfKernels(); i++) {

		TimedSDFactor* Actor = SDF->createActor();
		Actor->setName(SADF->getKernel(i)->getName());
		Actor->setType(SADF->getKernel(i)->getName());

		// Create Input Ports
				
		SDFport* Port;
		
		for (list<SADF_Channel*>::iterator j = SADF->getKernel(i)->getInputChannels().begin(); j != SADF->getKernel(i)->getInputChannels().end(); j++) {
			
			Port = Actor->createPort(SDFport::In, SADF->getKernel(i)->getConsumptionRate((*j)->getIdentity(), 0));
			Port->setName((CString)("In_") + (*j)->getName());
			
			if ((*j)->getBufferSize() != SADF_UNBOUNDED) {
				Port = Actor->createPort(SDFport::Out, SADF->getKernel(i)->getConsumptionRate((*j)->getIdentity(), 0)); 
				Port->setName((CString)("Reverse_Out_") + (*j)->getName());
			}
		}

		// Create Output Ports
		
		for (list<SADF_Channel*>::iterator j = SADF->getKernel(i)->getOutputChannels().begin(); j != SADF->getKernel(i)->getOutputChannels().end(); j++) {

			Port = Actor->createPort(SDFport::Out, SADF->getKernel(i)->getProductionRate((*j)->getIdentity(), SADF_DATA_CHANNEL, 0));
			Port->setName((CString)("Out_") + (*j)->getName());
			
			if ((*j)->getBufferSize() != SADF_UNBOUNDED) {
				Port = Actor->createPort(SDFport::In, SADF->getKernel(i)->getProductionRate((*j)->getIdentity(), SADF_DATA_CHANNEL, 0));
				Port->setName((CString)("Reverse_In_") + (*j)->getName());
			}		
		}

		// Create Ports for Self-Edge

		Port = Actor->createPort(SDFport::Out, 1);
		Port->setName("Self_Out");
		
		Port = Actor->createPort(SDFport::In, 1);
		Port->setName("Self_In");

		// Create Processor for Execution Time

		TimedSDFactor::Processor* Processor = Actor->addProcessor(SADF->getKernel(i)->getName());
		Processor->execTime = (SDFtime) SADF->getKernel(i)->getScenario(0)->getProfile(0)->getExecutionTime();
		Actor->setDefaultProcessor(SADF->getKernel(i)->getName());
	}
	
	// Create Channels

	for (CId i = 0; i != SADF->getNumberOfDataChannels(); i++) {
		
		SDFcomponent Component = new SDFcomponent(SDF, SDF->nrChannels());
		TimedSDFchannel* Channel = new TimedSDFchannel(Component);
		Channel->setName(SADF->getDataChannel(i)->getName());
		Channel->connectSrc(SDF->getActor(SADF->getDataChannel(i)->getSource()->getName())->getPort((CString)("Out_") + SADF->getDataChannel(i)->getName()));
		Channel->connectDst(SDF->getActor(SADF->getDataChannel(i)->getDestination()->getName())->getPort((CString)("In_") + SADF->getDataChannel(i)->getName()));
	
		if (SADF->getDataChannel(i)->getNumberOfInitialTokens() != 0)
			Channel->setInitialTokens(SADF->getDataChannel(i)->getNumberOfInitialTokens());
			
		Channel->setTokenSize(SADF->getDataChannel(i)->getTokenSize());
		
		SDF->addChannel(Channel);
		
		if (SADF->getDataChannel(i)->getBufferSize() != SADF_UNBOUNDED) {
			SDFcomponent Component = new SDFcomponent(SDF, SDF->nrChannels());
			TimedSDFchannel* ReverseChannel = new TimedSDFchannel(Component);
			ReverseChannel->setName((CString)("Reverse_") + SADF->getDataChannel(i)->getName());
			ReverseChannel->connectSrc(SDF->getActor(SADF->getDataChannel(i)->getDestination()->getName())->getPort((CString)("Reverse_Out_") + SADF->getDataChannel(i)->getName()));
			ReverseChannel->connectDst(SDF->getActor(SADF->getDataChannel(i)->getSource()->getName())->getPort((CString)("Reverse_In_") + SADF->getDataChannel(i)->getName()));
			ReverseChannel->setInitialTokens(SADF->getDataChannel(i)->getBufferSize() - SADF->getDataChannel(i)->getNumberOfInitialTokens());
			SDF->addChannel(ReverseChannel);
		}
	}

	for (CId i = 0; i != SADF->getNumberOfKernels(); i++) {
		SDFcomponent Component = new SDFcomponent(SDF, SDF->nrChannels());
		TimedSDFchannel* SelfChannel = new TimedSDFchannel(Component);
		SelfChannel->setName((CString)("Self_") + SADF->getKernel(i)->getName());
		SelfChannel->connectSrc(SDF->getActor(SADF->getKernel(i)->getName())->getPort("Self_Out"));
		SelfChannel->connectDst(SDF->getActor(SADF->getKernel(i)->getName())->getPort("Self_In"));
		SelfChannel->setInitialTokens(1);
		SDF->addChannel(SelfChannel);
	}	
	
	return SDF;
}
