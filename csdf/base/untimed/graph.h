/*
 *  TU Eindhoven
 *  Eindhoven, The Netherlands
 *
 *  Name            :   graph.h
 *
 *  Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
 *
 *  Date            :   July 13, 2005
 *
 *  Function        :   CSDF graph
 *
 *  History         :
 *      13-07-05    :   Initial version.
 *
 * $Id: graph.h,v 1.1.1.1 2007/10/02 10:59:49 sander Exp $
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

#ifndef CSDF_BASE_GRAPH_H_INCLUDED
#define CSDF_BASE_GRAPH_H_INCLUDED

#include "channel.h"

/**
 * CSDFgraph
 * Container for CSDF graph.
 */
class CSDFgraph : public CSDFcomponent
{
public:

    // Constructor
    CSDFgraph(CSDFcomponent &c);
    
    // Destructor
    virtual ~CSDFgraph();

    // Construct
    virtual CSDFgraph *create(CSDFcomponent &c) const;
    virtual CSDFgraph *createCopy(CSDFcomponent &c) const;
    virtual CSDFgraph *clone(CSDFcomponent &c) const;
    void construct(const CNodePtr graphNode);

    // Type
    CString getType() const { return type; };
    void setType(const CString &t) { type = t; };

    // Actors
    CSDFactor *getActor(const CId id);
    CSDFactor *getActor(const CString &name);
    CSDFactors &getActors() { return actors; };
    uint nrActors() const { return actors.size(); };
    CSDFactorsIter actorsBegin() { return actors.begin(); };
    CSDFactorsIter actorsEnd() { return actors.end(); };
    CSDFactorsCIter actorsBegin() const { return actors.begin(); };
    CSDFactorsCIter actorsEnd() const { return actors.end(); };
    void addActor(CSDFactor *a);
    void removeActor(const CString &name);
    virtual CSDFactor *createActor();
    virtual CSDFactor *createActor(CSDFcomponent &c);
        
    // Channels
    CSDFchannel *getChannel(const CId id);
    CSDFchannel *getChannel(const CString &name);
    CSDFchannels &getChannels() { return channels; };
    uint nrChannels() const { return channels.size(); };
    CSDFchannelsIter channelsBegin() { return channels.begin(); };
    CSDFchannelsIter channelsEnd() { return channels.end(); };
    CSDFchannelsCIter channelsBegin() const { return channels.begin(); };
    CSDFchannelsCIter channelsEnd() const { return channels.end(); };
    void addChannel(CSDFchannel *c);
    void removeChannel(const CString &name);
    virtual CSDFchannel *createChannel(CSDFcomponent &c);
    CSDFchannel *createChannel(CSDFactor *src, CSDFrate rateSrc, CSDFactor *dst,
                        CSDFrate rateDst);
    
    // Properties
    bool isConnected() const;
    bool isConsistent();
    
    typedef std::vector<int>                    RepetitionVector;
    typedef RepetitionVector::iterator          RepetitonVectorIter;
    typedef RepetitionVector::const_iterator    RepetitonVectorCIter;

    virtual RepetitionVector getRepetitionVector();

    // Print
    ostream &print(ostream &out);
    friend ostream &operator<<(ostream &out, CSDFgraph &g)
        { return g.print(out); };

private:
    // Repetition vector
    void calcFractionsConnectedActors(CFractions &fractions, CSDFactor *a,
                                            uint ratePeriod);
    RepetitionVector calcRepetitionVector(CFractions &fractions,
                                            uint ratePeriod);

    // Information
    CString type;
    
    // Actors and channels
    CSDFactors   actors;
    CSDFchannels channels;
        
};

typedef list<CSDFgraph*>            CSDFgraphs;
typedef CSDFgraphs::iterator        CSDFgraphsIter;
typedef CSDFgraphs::const_iterator  CSDFgraphsCIter;

/**
 * constructCSDFgraph ()
 * Construct an CSDF graph.
 */
CSDFgraph *constructCSDFgraph(const CNodePtr csdfNode);

#endif
