/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_graph.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Graph
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

#ifndef SADF_GRAPH_H_INCLUDED
#define SADF_GRAPH_H_INCLUDED

// Include type definitions

#include "sadf_process.h"
#include "sadf_channel.h"

// SADF_Graph Definition

class SADF_Graph : public SADF_Component {

public:
	// Constructors
	
	SADF_Graph(const CString &N, const CId ID);

	// Destructor

	virtual ~SADF_Graph();

	// Access to instance variables
	
	void setKernels(vector<SADF_Process*> K) { Kernels = K; };
	void setDetectors(vector<SADF_Process*> D) { Detectors = D; };
	void setDataChannels(vector<SADF_Channel*> C) { DataChannels = C; };
	void setControlChannels(vector<SADF_Channel*> C) { ControlChannels = C; };
	
	SADF_Process* getKernel(const CId ID) { return Kernels[ID]; };
	SADF_Process* getDetector(const CId ID) { return Detectors[ID]; };
	SADF_Channel* getDataChannel(const CId ID) { return DataChannels[ID]; };
	SADF_Channel* getControlChannel(const CId ID) { return ControlChannels[ID]; };

	SADF_Process* getKernel(const CString &KernelName);
	SADF_Process* getDetector(const CString &DetectorName);
	SADF_Channel* getChannel(const CString &ChannelName);

	CId getNumberOfKernels() const { return Kernels.size(); };
	CId getNumberOfDetectors() const { return Detectors.size(); };
	CId getNumberOfDataChannels() const { return DataChannels.size(); };
	CId getNumberOfControlChannels() const { return ControlChannels.size(); };
    
private:
	// Instance variables
	
	vector<SADF_Process*> Kernels;
	vector<SADF_Process*> Detectors;
	vector<SADF_Channel*> DataChannels;
	vector<SADF_Channel*> ControlChannels;
};

#endif
