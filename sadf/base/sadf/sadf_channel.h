/*
 *  Eindhoven University of Technology
 *  Eindhoven, The Netherlands
 *
 *  Name            :   sadf_channel.h
 *
 *  Author          :   Bart Theelen (B.D.Theelen@tue.nl)
 *
 *  Date            :   29 August 2006
 *
 *  Function        :   SADF Channel
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

#ifndef SADF_CHANNEL_H_INCLUDED
#define SADF_CHANNEL_H_INCLUDED

// Include type definitions

#include "sadf_component.h"

// Forward declarations

class SADF_Process;

// SADF_Channel Definition

class SADF_Channel : public SADF_Component {

public:
	// Constructor

	SADF_Channel(const CString &N, const CId ID, const CId T);
	
	// Destructor

	~SADF_Channel() { };

	// Access to instance variables
	
	void setSource(SADF_Process* S) { Source = S; };
	void setDestination(SADF_Process* D) { Destination = D; };
	
	void setBufferSize(const CId S) { BufferSize = S; };
	void setNumberOfInitialTokens(const CId N) { NumberOfInitialTokens = N; };
	void setTokenSize(const CId S) { TokenSize = S; };

	void addInitialTokens(const CId Number, const CId Scenario);
	
	SADF_Process* getSource() const { return Source; };
	SADF_Process* getDestination() const { return Destination; };

	CSize getBufferSize() const { return BufferSize; };
	CId getNumberOfInitialTokens() const { return NumberOfInitialTokens; };
	CId getTokenSize() const { return TokenSize; };
	
	CQueue& getContentQueue() { return ContentQueue; };
	CQueue& getNumbersQueue() { return NumbersQueue; };
	
private:
	// Instance variables

	SADF_Process* Source;
	SADF_Process* Destination;

	CSize BufferSize;
	CId NumberOfInitialTokens;
	CId TokenSize;
	
	CQueue ContentQueue;
	CQueue NumbersQueue;
};

#endif
