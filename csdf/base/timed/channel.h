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
 *  Function        :   Timed CSDF channel
 *
 *  History         :
 *      18-07-05    :   Initial version.
 *
 * $Id: channel.h,v 1.1.1.1 2007/10/02 10:59:49 sander Exp $
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

#ifndef CSDF_BASE_TIMED_CHANNEL_H_INCLUDED
#define CSDF_BASE_TIMED_CHANNEL_H_INCLUDED

#include "../untimed/graph.h"
#include "timed_types.h"

// Buffer size
typedef struct _CSDFbufferSize
{
    int sz;
    int src;
    int dst;
    int mem;
} CSDFbufferSize;

/**
 * TimedCSDFchannel
 * Timed CSDF channel.
 */
class TimedCSDFchannel : public CSDFchannel
{
public:

    // Constructor
    TimedCSDFchannel(CSDFcomponent &c);
        
    // Destructor
    ~TimedCSDFchannel();
 
     // Construct
    TimedCSDFchannel *create(CSDFcomponent &c) const;
    TimedCSDFchannel *createCopy(CSDFcomponent &c) const;
    TimedCSDFchannel *clone(CSDFcomponent &c) const;
   
    // Buffer sizes
    CSDFbufferSize getBufferSize() const { return bufferSize; };
    void setBufferSize(const CSDFbufferSize s) { bufferSize = s; };
    bool isUnbounded() const;
    TimedCSDFchannel *getStorageSpaceChannel() const 
            { return modelStorageSpaceCh; };
    void setStorageSpaceChannel(TimedCSDFchannel *c) 
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
    CSDFtime getMinLatency() const { return minLatency; };
    void setMinLatency(const CSDFtime t) { minLatency = t; };

    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, TimedCSDFchannel &c)
        { return c.print(out); };

private:
    TimedCSDFchannel *modelStorageSpaceCh;
    CSDFbufferSize bufferSize;
    CString tokenType;
    double minBandwidth;
    int tokenSize;
    CSDFtime minLatency;
};

#endif
