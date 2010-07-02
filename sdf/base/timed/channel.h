/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   channel.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 18, 2005
 *
 *  Function        :   Timed SDF channel
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: channel.h,v 1.1.1.1 2007/10/02 10:59:46 sander Exp $
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

#ifndef SDF_BASE_TIMED_CHANNEL_H_INCLUDED
#define SDF_BASE_TIMED_CHANNEL_H_INCLUDED

#include "../untimed/graph.h"
#include "timed_types.h"


/**
 * TimedSDFchannel
 * Timed SDF channel.
 */
class TimedSDFchannel : public SDFchannel
{
public:
    // Buffer size
    typedef struct _BufferSize
    {
        int sz;
        int src;
        int dst;
        int mem;
    } BufferSize;

public:
    // Constructor
    TimedSDFchannel(SDFcomponent &c);
        
    // Destructor
    ~TimedSDFchannel();
 
     // Construct
    TimedSDFchannel *create(SDFcomponent &c) const;
    TimedSDFchannel *createCopy(SDFcomponent &c) const;
    TimedSDFchannel *clone(SDFcomponent &c) const;

    // Properties
    void setProperties(const CNodePtr propertiesNode);
   
    // Buffer sizes
    BufferSize getBufferSize() const { return bufferSize; };
    void setBufferSize(const BufferSize s) { bufferSize = s; };
    bool isUnbounded() const;
    TimedSDFchannel *getStorageSpaceChannel() const 
            { return modelStorageSpaceCh; };
    void setStorageSpaceChannel(TimedSDFchannel *c) 
            { modelStorageSpaceCh = c; };
    bool modelsStorageSpace() const { return modelStorageSpaceCh != NULL; };
    
    // Bandwidth
    double getMinBandwidth() const { return minBandwidth; };
    void setMinBandwidth(const double b) { minBandwidth = b; };
    
    // Token size
    int getTokenSize() const { return tokenSize; };
    void setTokenSize(const int s) { tokenSize = s; };
    bool isControlToken() const;
    bool isDataToken() const;
    
    // Token type
    CString getTokenType() const { return tokenType; };
    void setTokenType(const CString &type) { tokenType = type; };

    // Latency
    SDFtime getMinLatency() const { return minLatency; };
    void setMinLatency(const SDFtime t) { minLatency = t; };

    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, TimedSDFchannel &c)
        { return c.print(out); };

private:
    TimedSDFchannel *modelStorageSpaceCh;
    BufferSize bufferSize;
    CString tokenType;
    double minBandwidth;
    int tokenSize;
    SDFtime minLatency;
};

#endif
